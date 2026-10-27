#include "bxm.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <fstream>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>

using namespace tinyxml2;

namespace bxm {
namespace {

constexpr UINT kShiftJis = 932;          // bxmEncoding = StringEncoding.shiftJis
constexpr uint16_t kNoString = 0xFFFF;
constexpr const char* kErrorString = "ERROR";

struct Header {
  char type[4];
  int32_t flags;
  uint16_t nodeCount;
  uint16_t dataCount;
  int32_t dataSize;
};
struct NodeInfo {
  int16_t childCount;
  int firstChildIndex;
  int16_t attributeCount;
  int16_t dataIndex;
};
struct DataOffsets {
  uint16_t nameOffset;
  uint16_t valueOffset;
};

// ---------------------------------------------------------------- strings

std::string SjisToUtf8(const std::string& s) {
  if (s.empty()) return {};
  int wl = MultiByteToWideChar(kShiftJis, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0);
  if (wl <= 0) return kErrorString;  // errorFallback
  std::wstring w(wl, L'\0');
  MultiByteToWideChar(kShiftJis, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), w.data(), wl);
  int ul = WideCharToMultiByte(CP_UTF8, 0, w.data(), wl, nullptr, 0, nullptr, nullptr);
  std::string u(ul, '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.data(), wl, u.data(), ul, nullptr, nullptr);
  return u;
}

std::string Utf8ToSjis(const std::string& s) {
  if (s.empty()) return {};
  int wl = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0);
  if (wl <= 0) throw std::runtime_error("invalid UTF-8 string: " + s);
  std::wstring w(wl, L'\0');
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), w.data(), wl);
  int sl = WideCharToMultiByte(kShiftJis, 0, w.data(), wl, nullptr, 0, nullptr, nullptr);
  std::string out(sl, '\0');
  BOOL usedDefault = FALSE;
  WideCharToMultiByte(kShiftJis, 0, w.data(), wl, out.data(), sl, nullptr, &usedDefault);
  if (usedDefault) throw std::runtime_error("string not representable in Shift-JIS: " + s);
  return out;
}

// ---------------------------------------------------------------- big endian IO

class Reader {
 public:
  explicit Reader(const std::vector<uint8_t>& d) : d_(d) {}
  size_t pos = 0;

  uint8_t ReadU8() {
    if (pos >= d_.size()) throw std::runtime_error("BXM: read out of range");
    return d_[pos++];
  }
  uint16_t ReadU16() { uint16_t hi = ReadU8(); return (uint16_t)((hi << 8) | ReadU8()); }
  int16_t ReadI16() { return (int16_t)ReadU16(); }
  int32_t ReadI32() {
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) v = (v << 8) | ReadU8();
    return (int32_t)v;
  }
  std::string zeroTerminated() {
    std::string s;
    for (uint8_t b; (b = ReadU8()) != 0;) s.push_back((char)b);
    return s;
  }

 private:
  const std::vector<uint8_t>& d_;
};

void PutU16(std::vector<uint8_t>& o, uint16_t v) { o.push_back(v >> 8); o.push_back(v & 0xFF); }
void PutI32(std::vector<uint8_t>& o, int32_t v) {
  for (int s = 24; s >= 0; s -= 8) o.push_back((uint8_t)((uint32_t)v >> s));
}

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  if (!f) throw std::runtime_error("cannot open file: " + p.string());
  return std::vector<uint8_t>(std::istreambuf_iterator<char>(f), {});
}

void WriteFileBytes(const std::filesystem::path& p, const void* data, size_t size) {
  std::ofstream f(p, std::ios::binary);
  if (!f) throw std::runtime_error("cannot write file: " + p.string());
  f.write((const char*)data, (std::streamsize)size);
}

// ---------------------------------------------------------------- reader

struct BxmXmlNode {
  std::string name, value;
  std::vector<std::pair<std::string, std::string>> attributes;  // ordem de inserção, chave duplicada sobrescreve
  std::vector<int> children;
  int firstChildIndex = -1;
  int childCount = -1;
};

XMLElement* NodeToXml(XMLDocument& doc, const std::vector<BxmXmlNode>& nodes, const BxmXmlNode& n) {
  XMLElement* e = doc.NewElement(n.name.c_str());
  if (!n.value.empty()) e->InsertEndChild(doc.NewText(n.value.c_str()));
  for (auto& [k, v] : n.attributes) e->SetAttribute(k.c_str(), v.c_str());
  for (int c : n.children) e->InsertEndChild(NodeToXml(doc, nodes, nodes[c]));
  return e;
}

// ---------------------------------------------------------------- writer helpers

std::string Trim(const std::string& s) {
  const char* ws = " \t\n\r\f\v";
  size_t b = s.find_first_not_of(ws);
  if (b == std::string::npos) return {};
  return s.substr(b, s.find_last_not_of(ws) - b + 1);
}

// XmlName.local (ignora prefixo "ns:")
std::string Local(const char* name) {
  std::string s = name;
  size_t i = s.find(':');
  return i == std::string::npos ? s : s.substr(i + 1);
}

// _getElementText: concatena só os nós de texto diretos (CDATA fica de fora)
std::string ElementText(const XMLElement* e) {
  std::string s;
  for (auto* c = e->FirstChild(); c; c = c->NextSibling())
    if (auto* t = c->ToText(); t && !t->CData()) s += t->Value();
  return s;
}

int AttributeCount(const XMLElement* e) {
  int n = 0;
  for (auto* a = e->FirstAttribute(); a; a = a->Next()) n++;
  return n;
}

int ChildElementCount(const XMLElement* e) {
  int n = 0;
  for (auto* c = e->FirstChildElement(); c; c = c->NextSiblingElement()) n++;
  return n;
}

void CollectNodes(const XMLElement* e, std::vector<const XMLElement*>& out) {
  out.push_back(e);
  for (auto* c = e->FirstChildElement(); c; c = c->NextSiblingElement()) CollectNodes(c, out);
}

// ---------------------------------------------------------------- pretty printer (toPrettyString: indent "\t")

void Escape(std::string& out, const char* s, bool attr) {
  for (; *s; ++s) {
    switch (*s) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '"': out += attr ? "&quot;" : "\""; break;
      case '\n': out += attr ? "&#xA;" : "\n"; break;
      case '\r': out += attr ? "&#xD;" : "\r"; break;
      case '\t': out += attr ? "&#x9;" : "\t"; break;
      case '>':
        if (!attr && out.size() >= 2 && out.compare(out.size() - 2, 2, "]]") == 0) out += "&gt;";
        else out += '>';
        break;
      default: out += *s;
    }
  }
}

// Elemento com nó de texto => conteúdo inteiro sem indentação (preserveWhitespace).
void PrintElement(std::string& out, const XMLElement* e, int level, bool compact) {
  if (!compact) out.append(level, '\t');
  out += '<';
  out += e->Name();
  for (auto* a = e->FirstAttribute(); a; a = a->Next()) {
    out += ' ';
    out += a->Name();
    out += "=\"";
    Escape(out, a->Value(), true);
    out += '"';
  }
  bool hasText = false, hasChild = false;
  for (auto* c = e->FirstChild(); c; c = c->NextSibling()) {
    if (c->ToText()) hasText = true;
    else if (c->ToElement()) hasChild = true;
  }
  if (!hasText && !hasChild) { out += "/>"; return; }
  out += '>';
  if (hasText || compact) {
    for (auto* c = e->FirstChild(); c; c = c->NextSibling()) {
      if (auto* t = c->ToText()) Escape(out, t->Value(), false);
      else if (auto* el = c->ToElement()) PrintElement(out, el, 0, true);
    }
  } else {
    for (auto* c = e->FirstChildElement(); c; c = c->NextSiblingElement()) {
      out += '\n';
      PrintElement(out, c, level + 1, false);
    }
    out += '\n';
    out.append(level, '\t');
  }
  out += "</";
  out += e->Name();
  out += '>';
}

}  // namespace

// ================================================================ parseBxm

XMLElement* ParseBxm(const std::vector<uint8_t>& bytes, XMLDocument& doc) {
  Reader r(bytes);

  Header header{};
  for (char& c : header.type) c = (char)r.ReadU8();
  header.flags = r.ReadI32();
  header.nodeCount = r.ReadU16();
  header.dataCount = r.ReadU16();
  header.dataSize = r.ReadI32();
  if (header.nodeCount == 0) throw std::runtime_error("BXM: no nodes");

  std::vector<NodeInfo> nodeInfos(header.nodeCount);
  for (auto& ni : nodeInfos) {
    ni.childCount = r.ReadI16();
    ni.firstChildIndex = r.ReadU16();
    ni.attributeCount = r.ReadI16();
    ni.dataIndex = r.ReadI16();
  }

  std::vector<DataOffsets> dataOffsets(header.dataCount);
  for (auto& d : dataOffsets) {
    d.nameOffset = r.ReadU16();
    d.valueOffset = r.ReadU16();
  }

  const size_t stringsOffsets = 0x10 + 8 * (size_t)header.nodeCount + 4 * (size_t)header.dataCount;
  auto readStr = [&](uint16_t offset) -> std::string {
    if (offset == kNoString) return {};
    r.pos = stringsOffsets + offset;
    return SjisToUtf8(r.zeroTerminated());
  };

  std::vector<BxmXmlNode> nodes(nodeInfos.size());
  for (size_t i = 0; i < nodeInfos.size(); i++) {
    const NodeInfo& info = nodeInfos[i];
    BxmXmlNode& node = nodes[i];
    node.firstChildIndex = info.firstChildIndex;
    node.childCount = info.childCount;

    node.name = readStr(dataOffsets.at(info.dataIndex).nameOffset);
    node.value = readStr(dataOffsets.at(info.dataIndex).valueOffset);

    for (int a = 0; a < info.attributeCount; a++) {
      const DataOffsets& d = dataOffsets.at(info.dataIndex + 1 + a);
      std::string attrName = readStr(d.nameOffset);
      std::string attrValue = readStr(d.valueOffset);
      bool found = false;
      for (auto& kv : node.attributes)
        if (kv.first == attrName) { kv.second = attrValue; found = true; break; }
      if (!found) node.attributes.emplace_back(std::move(attrName), std::move(attrValue));
    }
  }

  // getNodeChildren: primeiro filho + irmãos seguintes (até o firstChildIndex do primeiro filho)
  for (auto& node : nodes) {
    if (node.childCount == 0) continue;
    int first = node.firstChildIndex;
    if (first < 0 || first >= (int)nodes.size()) throw std::runtime_error("BXM: invalid child index");
    int end = nodes[first].firstChildIndex;
    if (end < first + 1 || end > (int)nodes.size()) throw std::runtime_error("BXM: invalid sibling range");
    node.children.push_back(first);
    for (int i = first + 1; i < end; i++) node.children.push_back(i);
  }

  XMLElement* root = NodeToXml(doc, nodes, nodes[0]);
  doc.InsertEndChild(root);
  return root;
}

// ================================================================ xmlToBxm

std::vector<uint8_t> XmlToBxm(const XMLElement* root) {
  // flatten tree
  std::vector<const XMLElement*> nodes;
  CollectNodes(root, nodes);

  // unique strings (tag names, tag text, attribute names and values) + offsets
  std::vector<std::string> uniqueStrings;  // já codificadas em Shift-JIS
  std::unordered_map<std::string, int> stringToOffset;
  int curOffset = 0;
  auto tryAddString = [&](const std::string& s) {
    if (s.empty() || stringToOffset.count(s)) return;
    if (curOffset >= kNoString) throw std::runtime_error("BXM: string table too large");
    std::string enc = Utf8ToSjis(s);
    stringToOffset[s] = curOffset;
    curOffset += (int)enc.size() + 1;
    uniqueStrings.push_back(std::move(enc));
  };
  for (auto* node : nodes) {
    tryAddString(Local(node->Name()));
    for (auto* a = node->FirstAttribute(); a; a = a->Next()) {
      tryAddString(Local(a->Name()));
      tryAddString(a->Value());
    }
    tryAddString(Trim(ElementText(node)));
  }
  auto offsetOf = [&](const std::string& s) -> uint16_t {
    auto it = stringToOffset.find(s);
    return it == stringToOffset.end() ? kNoString : (uint16_t)it->second;
  };

  // data offsets
  std::vector<DataOffsets> dataOffsets;
  std::unordered_map<const XMLElement*, int> nodeToDataIndex;
  for (auto* node : nodes) {
    nodeToDataIndex[node] = (int)dataOffsets.size();
    dataOffsets.push_back({offsetOf(Local(node->Name())), offsetOf(Trim(ElementText(node)))});
    for (auto* a = node->FirstAttribute(); a; a = a->Next())
      dataOffsets.push_back({offsetOf(Local(a->Name())), offsetOf(a->Value())});
  }

  // node infos na ordem correta: raiz; filhos; depois recursivamente os filhos de cada filho
  std::vector<NodeInfo> nodeInfos;
  std::vector<const XMLElement*> infoNodes;
  std::unordered_map<const XMLElement*, int> nodeToInfoIndex;
  auto addInfo = [&](const XMLElement* e) {
    nodeToInfoIndex[e] = (int)nodeInfos.size();
    nodeInfos.push_back({(int16_t)ChildElementCount(e), -1, (int16_t)AttributeCount(e),
                         (int16_t)nodeToDataIndex[e]});
    infoNodes.push_back(e);
  };
  std::function<void(const XMLElement*)> addChildren = [&](const XMLElement* n) {
    for (auto* c = n->FirstChildElement(); c; c = c->NextSiblingElement()) addInfo(c);
    for (auto* c = n->FirstChildElement(); c; c = c->NextSiblingElement()) addChildren(c);
  };
  addInfo(root);
  addChildren(root);

  // first child index / next sibling index
  for (size_t i = 0; i < nodeInfos.size(); i++) {
    const XMLElement* e = infoNodes[i];
    if (nodeInfos[i].childCount > 0) {
      nodeInfos[i].firstChildIndex = nodeToInfoIndex[e->FirstChildElement()];
    } else if (e != root) {
      auto* parent = e->Parent()->ToElement();
      nodeInfos[i].firstChildIndex = nodeToInfoIndex[parent->LastChildElement()] + 1;
    } else {
      nodeInfos[i].firstChildIndex = (int)nodeInfos.size();
    }
  }

  if (nodeInfos.size() > 0xFFFF || dataOffsets.size() > 0xFFFF)
    throw std::runtime_error("BXM: too many nodes/attributes");

  // write file
  std::vector<uint8_t> out;
  out.insert(out.end(), {'X', 'M', 'L', 0});
  PutI32(out, 0);                          // flags
  PutU16(out, (uint16_t)nodeInfos.size());
  PutU16(out, (uint16_t)dataOffsets.size());
  PutI32(out, curOffset);                  // dataSize = soma de (len + 1)
  for (auto& ni : nodeInfos) {
    PutU16(out, (uint16_t)ni.childCount);
    PutU16(out, (uint16_t)ni.firstChildIndex);
    PutU16(out, (uint16_t)ni.attributeCount);
    PutU16(out, (uint16_t)ni.dataIndex);
  }
  for (auto& d : dataOffsets) {
    PutU16(out, d.nameOffset);
    PutU16(out, d.valueOffset);
  }
  for (auto& s : uniqueStrings) {
    out.insert(out.end(), s.begin(), s.end());
    out.push_back(0);
  }
  return out;
}

// ================================================================ file helpers

void ConvertBxmFileToXml(const std::filesystem::path& bxmPath, const std::filesystem::path& xmlPath) {
  XMLDocument doc;
  XMLElement* root = ParseBxm(ReadFileBytes(bxmPath), doc);
  std::string xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  PrintElement(xml, root, 0, false);
  xml += '\n';
  WriteFileBytes(xmlPath, xml.data(), xml.size());
}

void ConvertXmlToBxmFile(const std::filesystem::path& xmlPath, const std::filesystem::path& bxmPath) {
  auto bytes = ReadFileBytes(xmlPath);
  XMLDocument doc;
  if (doc.Parse((const char*)bytes.data(), bytes.size()) != XML_SUCCESS)
    throw std::runtime_error(std::string("XML parse error: ") + doc.ErrorStr());
  const XMLElement* root = doc.RootElement();
  if (!root) throw std::runtime_error("XML has no root element");
  auto out = XmlToBxm(root);
  WriteFileBytes(bxmPath, out.data(), out.size());
}

}  // namespace bxm

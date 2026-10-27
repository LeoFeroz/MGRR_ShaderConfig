#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>
#include "tinyxml2.h"

namespace bxm {

tinyxml2::XMLElement* ParseBxm(const std::vector<uint8_t>& bytes, tinyxml2::XMLDocument& doc);

std::vector<uint8_t> XmlToBxm(const tinyxml2::XMLElement* root);

void ConvertBxmFileToXml(const std::filesystem::path& bxmPath, const std::filesystem::path& xmlPath);
void ConvertXmlToBxmFile(const std::filesystem::path& xmlPath, const std::filesystem::path& bxmPath);

}  // namespace bxm

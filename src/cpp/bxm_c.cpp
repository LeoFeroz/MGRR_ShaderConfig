#include "bxm_c.h"
#include "bxm.h"
#include <string>

static std::string g_error;

extern "C" int bxm_to_xml(const char* bxm_path, const char* xml_path) {
    try {
        bxm::ConvertBxmFileToXml(bxm_path, xml_path);
        return 0;
    } catch (const std::exception& e) {
        g_error = e.what();
        return -1;
    } catch (...) {
        g_error = "unknown error";
        return -1;
    }
}

extern "C" int xml_to_bxm(const char* xml_path, const char* bxm_path) {
    try {
        bxm::ConvertXmlToBxmFile(xml_path, bxm_path);
        return 0;
    } catch (const std::exception& e) {
        g_error = e.what();
        return -1;
    } catch (...) {
        g_error = "unknown error";
        return -1;
    }
}

extern "C" const char* bxm_last_error(void) {
    return g_error.c_str();
}
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int bxm_to_xml(const char* bxm_path, const char* xml_path);
int xml_to_bxm(const char* xml_path, const char* bxm_path);

const char* bxm_last_error(void);

#ifdef __cplusplus
}
#endif
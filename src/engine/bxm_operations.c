#include <direct.h>
#include "definitions.h"
#include "bxm_c.h"

extern char out_dir[];

int Convert_BXM_XML(const char* file_path) {
    char xml_path[256];

    _mkdir(out_dir);
    snprintf(xml_path, sizeof(xml_path), "%s/shadersetting.xml", out_dir);

    if (bxm_to_xml(file_path, xml_path) != 0) {
        printf("BXM error: %s\n", bxm_last_error());
        return -1;
    }

    printf("Created: %s\n", xml_path);
    return 0;
}

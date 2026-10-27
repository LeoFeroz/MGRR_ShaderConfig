#include <direct.h>
#include "definitions.h"

char inp_dir[20] = "input";
char out_dir[20] = "output";

static char tools_dir[20] = "tools";
static char add_shaders_dir[20] = "add_shaders";

static const char* file_names[4] = {
    "ShaderSign.dat",
    "shadersetting.bxm",
    "shader.dat",
    "shader2.dat", //VSO Only
};

static const char* externaltools[1] = {
    "platinum_dat_tool.exe",
};


void ReadBinnaryFiles(void) {
    char file_path[256];
    char tools_path[256];
    char folder_path[256];
    char sign_xml[256];
    char setting_xml[256];
    int sign_ok = 0, setting_ok = 0;

    snprintf(tools_path, sizeof(tools_path), "%s\\%s", tools_dir, externaltools[0]);
    snprintf(folder_path, sizeof(folder_path), "%s\\shader_dat", inp_dir);
    snprintf(sign_xml, sizeof(sign_xml), "%s/ShaderSign.xml", out_dir);
    snprintf(setting_xml, sizeof(setting_xml), "%s/shadersetting.xml", out_dir);

    for (i8 i = 0; i < 4; i++) {
        snprintf(file_path, sizeof(file_path), "%s/%s", inp_dir, file_names[i]);

        FILE* file = fopen(file_path, "rb");

        if (file == NULL)
        {
            printf("File not found: %s\n", file_path);
            continue;
        }

        switch (i) {
            case 0: {
                char xml_path[256];
                snprintf(xml_path, sizeof(xml_path), "%s/ShaderSign.xml", out_dir);
                _mkdir(out_dir);

                FILE* xml = fopen(xml_path, "w");
                if (xml == NULL) {
                    printf("Cannot create: %s\n", xml_path);
                    break;
                }
                ReadShaderSign(file, xml);
                fclose(xml);
                printf("Created: %s\n", xml_path);
                sign_ok = 1;
                break;
            }
            case 1:
                setting_ok = (Convert_BXM_XML(file_path) == 0);
                break;
            case 2:
                dat_unpack(file_path, tools_path);
                break;
        }

        fclose(file);
    }

    /* Funde os dois XML em ShaderConfig.xml, na pasta base do exe */
    if (sign_ok && setting_ok) {
        if (Merge_ShaderXML(sign_xml, setting_xml, "ShaderConfig.xml") == 0)
            printf("Created: ShaderConfig.xml\n");
    }

}

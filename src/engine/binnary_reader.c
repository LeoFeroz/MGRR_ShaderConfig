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
	i8 i = 0;

    snprintf(tools_path, sizeof(tools_path), "%s\\%s", tools_dir, externaltools[0]);

    snprintf(folder_path, sizeof(folder_path), "%s\\shader_dat", inp_dir);

    for (i8 i = 0; i < 4; i++) {
        snprintf(file_path, sizeof(file_path), "%s/%s", inp_dir, file_names[i]);

        FILE* file = fopen(file_path, "rb");

        if (file == NULL)
        {
            printf("File not found: %s\n", file_path);
            continue;
        }

        switch (i) {
            case 0:
                ReadShaderSign(file);
                break;
            case 2:
                dat_unpack(file_path, tools_path);
                break;
        }

        fclose(file);
    }

}

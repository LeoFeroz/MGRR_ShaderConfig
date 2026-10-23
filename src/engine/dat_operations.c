#include "definitions.h"

extern char inp_dir[];

void dat_unpack(const char* input_path, const char* tool_path) {
    char command[1024];
    char del_path[256];

    snprintf(
        command,
        sizeof(command),
        "\"\"%s\" \"%s\"\"",
        tool_path,
        input_path
    );

    system(command);

    snprintf(del_path, sizeof(del_path), "%s\\shader_dat\\kwasinfo.xml", inp_dir);
    remove(del_path);
}

void dat_repack(const char* folder_path, const char* tool_path) {
    char command[1024];

    snprintf(
        command,
        sizeof(command),
        "\"\"%s\" \"%s\"\"",
        tool_path,
        folder_path
    );

    system(command);
}
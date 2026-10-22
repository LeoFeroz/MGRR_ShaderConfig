#include "definitions.h"

static char inp_dir[20] = "input";
static char out_dir[20] = "output";

static char tools_dir[20] = "tools";
static char add_shaders_dir[20] = "add_shaders";

static const char* file_names[4] = {
    "ShaderSign.dat",
    "shadersetting.bxm",
    "shader.dat",
    "shader2.dat", //VSO Only
};

const char* FLag_Names_1[] = {
        "UnkParameter",
        "RenderMode",
        "UnkParameter",
        "UnkParameter",
};

const char* FLag_Names_Unk[] = {
        "UnkParameter",
        "UnkParameter",
        "UnkParameter",
        "UnkParameter",
};

const char* UnkParameters[MAX_BITS] = {
    "flagA",
    "flagB",
    "flagC",
    "flagD",
    "flagE",
    "flagF",
    "flagG",
    "flagH",
};

const char* RenderMode[MAX_BITS] = {
    "flagUnk0",
    "depthB1",
    "depthB0",
    "AlphaBlend",
    "AlphaTest",
    "DisableReflex",
    "flagUnk6",
    "Base",
};

const char** GroupsFlagsIdentifiers[4] = {
    UnkParameters,
    RenderMode,
    UnkParameters,
    UnkParameters,
};

const char** UnkGroupsFlagsIdentifiers[4] = {
    UnkParameters,
    UnkParameters,
    UnkParameters,
    UnkParameters,
};

void ReadBinnaryFiles(void) {
    char file_path[256];
	i8 i = 0;

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
        }

        fclose(file);
    }

}

void BitFlagOperations(FILE* file, GenericBitFlag* bitflag, const char** names) {

    fread(&bitflag->raw, sizeof(bitflag->raw), 1, file);
    for (int i = 0; i < MAX_BITS; i++) {
        printf("            <%s = %s>\n", names[i], BOOL_STR((bitflag->raw >> i) & 1u));
    }
}

void GroupShaderSign(FILE* file, u8 index, const char** FlagNames, const char** IdentifiersNames) {
    GenericBitFlag bits = { 0 };
    printf("      <Configuration>\n");
    printf("        <%s>\n", FlagNames[index]);
    BitFlagOperations(file, &bits, IdentifiersNames[index]);
    printf("        </%s>\n", FlagNames[index]);
    printf("      </Configuration>\n");
}

void ReadShaderSign(FILE* file) {
	printf("  <Data_00>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(file, i, FLag_Names_1, GroupsFlagsIdentifiers);
    }
    printf("  </Data_00>\n");
    printf("  <Data_01>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(file, i, FLag_Names_Unk, UnkGroupsFlagsIdentifiers);
    }
    printf("  </Data_01>\n");

}

void ReadShaderSign(FILE* file) {
    printf("  <Data_00>\n");
    for (u8 i = 0; i < 2; i++) {
        GroupShaderSign(file, i, FLag_Names_1, GroupsFlagsIdentifiers);
    }

    printf("  </Data_01>\n");

}
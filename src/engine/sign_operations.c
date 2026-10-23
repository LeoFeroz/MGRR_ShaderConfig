#include "definitions.h"

const char* FLag_Names_1[] = {
        "UnkParameter",
        "RenderMode",
        "UnkParameter",
        "UnkParameter",
};

const char* FLag_Names_2[] = {
        "UnkParameter",
        "ShaderParameter",
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


void ReadShaderSign(FILE* input) {
    int i = 0;

    printf("<ShaderSign>\n");
    for (i = 0; i < GetEntryCount(input); i++) {
        printf("  <Work> //%d\n", i);
        WorkHeader(input);
        printf("  </Work>\n");
        fseek(input, 0x1C, SEEK_CUR);
    }
    printf("</ShaderSign>\n");

    printf("\n");
    printf("Total Works = %d", i);
    printf("\n");
}

int GetEntryCount(FILE* input) {
    long pos = ftell(input);

    fseek(input, 0, SEEK_END);
    long size = ftell(input);

    fseek(input, pos, SEEK_SET);

    return (int)(size / 0x28);
}

void WorkHeader(FILE* input) {
    printf("    <Data_01>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, i, FLag_Names_1, GroupsFlagsIdentifiers);
    }
    printf("    </Data_01>\n");
    printf("    <Data_02>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, i, FLag_Names_2, UnkGroupsFlagsIdentifiers);
    }
    printf("    </Data_02>\n");
    printf("    <Data_03>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, i, FLag_Names_Unk, UnkGroupsFlagsIdentifiers);
    }
    printf("    </Data_03>\n");

}

void GroupShaderSign(FILE* input, u8 index, const char** FlagNames, const char** IdentifiersNames) {
    GenericBitFlag bits = { 0 };
    printf("      <Configuration>\n");
    printf("        <%s>\n", FlagNames[index]);
    BitFlagOperations(input, &bits, IdentifiersNames[index]);
    printf("        </%s>\n", FlagNames[index]);
    printf("      </Configuration>\n");
}

void BitFlagOperations(FILE* input, GenericBitFlag* bitflag, const char** names) {

    fread(&bitflag->raw, sizeof(bitflag->raw), 1, input);
    for (int i = 0; i < MAX_BITS; i++) {
        printf("          <%s = %s>\n", names[i], BOOL_STR((bitflag->raw >> i) & 1u));
    }
}
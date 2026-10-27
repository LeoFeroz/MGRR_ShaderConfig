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


void ReadShaderSign(FILE* input, FILE* out) {
    int count = GetEntryCount(input);

    fprintf(out, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(out, "<ShaderSign count=\"%d\">\n", count);
    for (int i = 0; i < count; i++) {
        fprintf(out, "  <Work index=\"%d\">\n", i);
        WorkHeader(input, out);
        fseek(input, 0x1C, SEEK_CUR);  /* padding nao usado pelo jogo; o repack preenche com zeros */
        fprintf(out, "  </Work>\n");
    }
    fprintf(out, "</ShaderSign>\n");

    printf("Total Works = %d\n", count);
}

int GetEntryCount(FILE* input) {
    long pos = ftell(input);

    fseek(input, 0, SEEK_END);
    long size = ftell(input);

    fseek(input, pos, SEEK_SET);

    return (int)(size / 0x28);
}

void WorkHeader(FILE* input, FILE* out) {
    fprintf(out, "    <Data_01>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, out, i, FLag_Names_1, GroupsFlagsIdentifiers);
    }
    fprintf(out, "    </Data_01>\n");
    fprintf(out, "    <Data_02>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, out, i, FLag_Names_2, UnkGroupsFlagsIdentifiers);
    }
    fprintf(out, "    </Data_02>\n");
    fprintf(out, "    <Data_03>\n");
    for (u8 i = 0; i < 4; i++) {
        GroupShaderSign(input, out, i, FLag_Names_Unk, UnkGroupsFlagsIdentifiers);
    }
    fprintf(out, "    </Data_03>\n");
}

void GroupShaderSign(FILE* input, FILE* out, u8 index, const char** FlagNames, const char*** IdentifiersNames) {
    GenericBitFlag bits = { 0 };
    fprintf(out, "      <%s>", FlagNames[index]);
    BitFlagOperations(input, out, &bits, IdentifiersNames[index]);
    fprintf(out, "</%s>\n", FlagNames[index]);
}

/* Escreve as 8 flags na mesma linha: flagA=False, flagB=True, ... */
void BitFlagOperations(FILE* input, FILE* out, GenericBitFlag* bitflag, const char** names) {
    if (fread(&bitflag->raw, sizeof(bitflag->raw), 1, input) != 1) {
        printf("Warning: could not read flags\n");
        bitflag->raw = 0;
    }
    for (int i = 0; i < MAX_BITS; i++) {
        fprintf(out, "%s%s=%s", i ? ", " : "", names[i], BOOL_STR((bitflag->raw >> i) & 1u));
    }
}

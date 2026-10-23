#pragma once

#include "datatype.h"

void ReadBinnaryFiles(void);

void ReadShaderSign(FILE* input);

int GetEntryCount(FILE* input);

void WorkHeader(FILE* input);

void GroupShaderSign(FILE* input, u8 index, const char** FlagNames, const char** IdentifiersNames);

void BitFlagOperations(FILE* input, GenericBitFlag* bitflag, const char** names);

void dat_unpack(const char* input_path, const char* tool_path);

void dat_repack(const char* folder_path, const char* tool_path);


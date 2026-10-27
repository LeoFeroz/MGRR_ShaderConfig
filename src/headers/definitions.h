#pragma once

#include "datatype.h"

/*
  SIGN Operations
*/ 
void ReadBinnaryFiles(void);
void ReadShaderSign(FILE* input, FILE* out);
int GetEntryCount(FILE* input);
void WorkHeader(FILE* input, FILE* out);
void GroupShaderSign(FILE* input, FILE* out, u8 index, const char** FlagNames, const char*** IdentifiersNames);
void BitFlagOperations(FILE* input, FILE* out, GenericBitFlag* bitflag, const char** names);
/*
  Dat Operations
*/
void dat_unpack(const char* input_path, const char* tool_path);
void dat_repack(const char* folder_path, const char* tool_path);
/*
  BXM Operations
*/
int Convert_BXM_XML(const char* file_path);

/*
  Merge Operations
*/
int Merge_ShaderXML(const char* sign_xml, const char* setting_xml, const char* merged_xml);

/*
  Split / Repack Operations (WORK IN PROGRESS)
*/
int Split_ShaderXML(const char* merged_xml, const char* sign_xml, const char* setting_xml);
int ParseFlagsLine(const char* text, const char** names, u8* out_raw);
int ShaderSignXML_To_Dat(const char* sign_xml, const char* dat_path);

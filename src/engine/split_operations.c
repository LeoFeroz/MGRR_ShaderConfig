#include "definitions.h"

/*
  Split / Repack Operations  --  AINDA NAO IMPLEMENTADO (apenas esqueleto)

  Fluxo planejado do --Repack:

    ShaderConfig.xml  --Split_ShaderXML-->  output/ShaderSign.xml
                                            output/shadersetting.xml
    output/ShaderSign.xml      --ShaderSignXML_To_Dat-->  ShaderSign.dat
    output/shadersetting.xml   --xml_to_bxm (bxm_c.h)-->  shadersetting.bxm
*/

/*
  Le o ShaderConfig.xml (fundido) e separa cada <Work> de volta nos dois XML originais:
    - ShaderSign.xml     : somente <Data_01>, <Data_02>, <Data_03> de cada Work
    - shadersetting.xml  : somente os campos do shadersetting (<No>, <shader_name>,
                           <use_vsoname>, <use_psoname>, <use_psoname_ev>, <shader_vso>,
                           <shader_pso>, <shader_pso_ev> e, quando existir, <rain_shader>),
                           dentro de <root><Work>...</Work></root>

  Retorna 0 em sucesso, -1 em erro.
*/
int Split_ShaderXML(const char* merged_xml, const char* sign_xml, const char* setting_xml) {
    (void)merged_xml;
    (void)sign_xml;
    (void)setting_xml;
    /* TODO */
    return -1;
}

/*
  Converte a linha de flags "flagA=False, flagB=True, ..." no byte correspondente.
  names = lista de 8 nomes do grupo (na ordem dos bits 0..7).
  Retorna 0 em sucesso, -1 se algum nome estiver faltando/desconhecido.
*/
int ParseFlagsLine(const char* text, const char** names, u8* out_raw) {
    (void)text;
    (void)names;
    (void)out_raw;
    /* TODO */
    return -1;
}

/*
  Gera o ShaderSign.dat a partir do ShaderSign.xml:
  por Work, 12 bytes de flags (Data_01..03 x 4 grupos) + 0x1C bytes zerados (0x28 no total).

  Retorna 0 em sucesso, -1 em erro.
*/
int ShaderSignXML_To_Dat(const char* sign_xml, const char* dat_path) {
    (void)sign_xml;
    (void)dat_path;
    /* TODO */
    return -1;
}

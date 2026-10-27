#include "definitions.h"

/*
  Funde ShaderSign.xml (flags de cada Work) com shadersetting.xml (parametros de cada Work).
  Os Works sao casados pela posicao (o 1o Work de um com o 1o do outro, etc.).
  Os parametros do shadersetting entram no FINAL de cada <Work> do ShaderSign.

  Retorna 0 em sucesso, -1 em erro.
*/

typedef struct {
    char** lines;
    int    count;
    int    cap;
} LineList;

static void list_free(LineList* l) {
    for (int i = 0; i < l->count; i++) free(l->lines[i]);
    free(l->lines);
    l->lines = NULL;
    l->count = l->cap = 0;
}

static int list_add(LineList* l, const char* s) {
    if (l->count == l->cap) {
        int ncap = l->cap ? l->cap * 2 : 16;
        char** n = (char**)realloc(l->lines, ncap * sizeof(char*));
        if (!n) return -1;
        l->lines = n;
        l->cap = ncap;
    }
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return -1;
    memcpy(copy, s, len + 1);
    l->lines[l->count++] = copy;
    return 0;
}

/* Le uma linha de qualquer tamanho; devolve ponteiro para buffer (realocado) ou NULL no fim. */
static char* read_line(FILE* f, char** buf, size_t* cap) {
    size_t len = 0;
    int ch;

    while ((ch = fgetc(f)) != EOF) {
        if (len + 2 > *cap) {
            size_t ncap = *cap ? *cap * 2 : 256;
            char* nb = (char*)realloc(*buf, ncap);
            if (!nb) return NULL;
            *buf = nb;
            *cap = ncap;
        }
        if (ch == '\n') break;
        (*buf)[len++] = (char)ch;
    }
    if (ch == EOF && len == 0) return NULL;

    while (len > 0 && ((*buf)[len - 1] == '\r')) len--;
    (*buf)[len] = '\0';
    return *buf;
}

static const char* trim_left(const char* s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

/* Le o shadersetting.xml: guarda, para cada <Work>, as linhas filhas (sem indentacao). */
static int load_setting(const char* path, LineList** works, int* work_count) {
    FILE* f = fopen(path, "r");
    char* buf = NULL;
    size_t cap = 0;
    int in_work = 0, n = 0, wcap = 0;
    LineList* w = NULL;

    if (!f) {
        printf("Merge: cannot open %s\n", path);
        return -1;
    }

    while (read_line(f, &buf, &cap)) {
        const char* t = trim_left(buf);

        if (strcmp(t, "<Work>") == 0) {
            if (n == wcap) {
                int ncap = wcap ? wcap * 2 : 64;
                LineList* nw = (LineList*)realloc(w, ncap * sizeof(LineList));
                if (!nw) goto fail;
                w = nw;
                wcap = ncap;
            }
            memset(&w[n], 0, sizeof(LineList));
            n++;
            in_work = 1;
        } else if (strcmp(t, "</Work>") == 0) {
            in_work = 0;
        } else if (in_work && *t) {
            if (list_add(&w[n - 1], t) != 0) goto fail;
        }
    }

    free(buf);
    fclose(f);
    *works = w;
    *work_count = n;
    return 0;

fail:
    printf("Merge: out of memory\n");
    free(buf);
    fclose(f);
    for (int i = 0; i < n; i++) list_free(&w[i]);
    free(w);
    return -1;
}

int Merge_ShaderXML(const char* sign_xml, const char* setting_xml, const char* merged_xml) {
    LineList* works = NULL;
    int work_count = 0, sign_works = 0, index = 0, result = -1;
    FILE* in = NULL;
    FILE* out = NULL;
    char* buf = NULL;
    size_t cap = 0;

    if (load_setting(setting_xml, &works, &work_count) != 0) return -1;

    in = fopen(sign_xml, "r");
    if (!in) {
        printf("Merge: cannot open %s\n", sign_xml);
        goto done;
    }

    /* 1a passada: conta os Works do ShaderSign para validar antes de escrever */
    while (read_line(in, &buf, &cap)) {
        if (strcmp(trim_left(buf), "</Work>") == 0) sign_works++;
    }
    if (sign_works != work_count) {
        printf("Merge: Work count mismatch (ShaderSign=%d, shadersetting=%d)\n", sign_works, work_count);
        goto done;
    }
    rewind(in);

    out = fopen(merged_xml, "w");
    if (!out) {
        printf("Merge: cannot create %s\n", merged_xml);
        goto done;
    }

    while (read_line(in, &buf, &cap)) {
        if (strcmp(trim_left(buf), "</Work>") == 0 && index < work_count) {
            for (int i = 0; i < works[index].count; i++)
                fprintf(out, "    %s\n", works[index].lines[i]);
            index++;
        }
        fprintf(out, "%s\n", buf);
    }

    result = 0;

done:
    if (in) fclose(in);
    if (out) fclose(out);
    free(buf);
    for (int i = 0; i < work_count; i++) list_free(&works[i]);
    free(works);
    return result;
}

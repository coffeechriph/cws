#ifndef CWSSTRING_H
#define CWSSTRING_H
#include "stdlib.h"
#include "../types.h"
#include <string.h>
#include "log.h"

typedef struct { u32 length; char *data;} cws_str;

char* copy_conststr(const char *src, i32 src_length);
void cws_str_init(cws_str *s);
void cws_str_build(cws_str *s, const char *s2);
void cws_str_append(cws_str *s, const char *s2);
void cws_str_copy(cws_str *dest, cws_str *src);
void cws_str_free(cws_str* s);
bool cws_str_cmp(cws_str *s1, cws_str *s2);
#endif
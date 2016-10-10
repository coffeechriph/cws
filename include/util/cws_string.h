#ifndef CWSSTRING_H
#define CWSSTRING_H
#include "stdlib.h"
#include "../types.h"
#include "log.h"

typedef struct { u32 length; char *data;} cws_string;
#define cws_string() (cws_string){.length = 0,.data = NULL};

void cws_string_build(cws_string *s, const char *s2);
void cws_string_copy(cws_string *dest, cws_string *src);
void cws_string_free(cws_string* s);
#endif
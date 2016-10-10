#ifndef FILE_H
#define FILE_H
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "limits.h"
#include "../util/log.h"
#include "../types.h"

char *cwsReadFile(const char *file, i32 *length);
bool cwsOpenFile(const char *file);
bool cwsCloseFile(const char *file);
#endif
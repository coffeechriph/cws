#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../types.h"
#if !defined(NDEBUG)
#define cws_log(fmt, ...) {															\
							printf("%d [%s]: ", __LINE__, __func__);				\
							printf(fmt, ##__VA_ARGS__);								\
							printf("\n");											\
							cws_log_print(fmt, __LINE__, __func__, ##__VA_ARGS__);	\
						 }
#else
#define cws_log(fmt, ...)
#endif

void cws_log_print(const char *fmt, i32 LINE, const char *FUNC, ...);
bool cws_log_open(const char *file);
bool cws_log_close();
#endif
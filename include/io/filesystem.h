#ifndef FILESYSTEM
#define FILESYSTEM
#ifdef linux
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#endif

#include "../util/cws_string.h"
typedef enum
{
    FT_REGULAR,
    FT_DIR
} FILE_TYPE;

typedef struct
{
    cws_string name;
    i32 file_type;
} cwsDir;

bool isDirectory(const char *path);
bool isFile(const char *path);

char* getFileExt(const char *path);
char* getFileName(const char *path);
char* getFullFileName(const char *path);
#endif

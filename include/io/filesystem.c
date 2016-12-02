#include "filesystem.h"

#ifdef linux
bool isDirectory(const char *path)
{
    struct stat st;
    stat(path, &st);
    
    return S_ISDIR(st.st_mode);
}

bool isFile(const char *path)
{
    struct stat st;
    stat(path, &st);
    
    return S_ISREG(st.st_mode);
}

char* getFileExt(const char *path)
{
    if(!isFile(path))
    {
        cws_log("%s is not a file!", path);
        return NULL;
    }
    
     size_t len = strlen(path);
    char buffer[32];
    i32 off = 0;
    bool is_ext = false;
    for(i32 i = len-1; i > 0; --i)
    {
        if(path[i] == '.')
        {
            is_ext = true;
            break;
        }
        
        buffer[off++] = path[i];
    }
    
    if(is_ext)
    {
        char *ext = malloc(sizeof(char) * off+1);
    for(i32 i = 0; i < off; ++i)
    {
        ext[i] = buffer[off-1-i];
    }
    ext[off] = '\0';
    return ext;
}

return NULL;
}

char* getFileName(const char *path)
{
    if(!isFile(path) && !isDirectory(path))
    {
        cws_log("%s is not a file or directory!", path);
        return NULL;
    }
    
    size_t len = strlen(path);
    char buffer[64];
    i32 off = 0;
    for(i32 i = len-1; i > 0; --i)
    {
        if(path[i] == '/' || path[i] == '\\')
        {
            break;
        }
        else
        {
            buffer[off++] = path[i];
        }
    }
    
    char *name = malloc(sizeof(char) * off+1);
    for(i32 i = 0; i < off; ++i)
    {
        name[i] = buffer[off-1-i];
    }
    name[off] = '\0';
    
    return name;
}

char* getFullFileName(const char *path)
{
    if(!isFile(path) && !isDirectory(path))
    {
        cws_log("%s is not a file or directory!", path);
        return NULL;
    }
    
    char *rp = realpath(path, NULL);
    return rp;
}

#endif

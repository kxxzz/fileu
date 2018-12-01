#include "fileu.h"
#include <malloc.h>
#include <string.h>
#include <assert.h>


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#endif


#include <sys/stat.h>
#ifdef _WIN32
# define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
# define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))


#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))








void fileu_getDirName(char* dir, const char* path, u32 tbufSize)
{
    u32 len = (u32)strlen(path);
    len = min(len, tbufSize - 1);
    strncpy(dir, path, len + 1);
    for (char* p = dir + len - 1; p != dir; --p)
    {
        char c = *p;
        if (('\\' == c) || ('/' == c))
        {
            *p = 0;
            break;
        }
    }
}

void fileu_getLocalFileName(char* filename, const char* path, u32 tbufSize)
{
    u32 len = (u32)strlen(path);
    len = min(len, tbufSize - 1);
    for (const char* p = path + len - 1; p != path; --p)
    {
        char c = *p;
        if (('\\' == c) || ('/' == c))
        {
            strncpy(filename, p + 1, len - (p - path));
            return;
        }
    }
    strncpy(filename, path, len + 1);
}

void fileu_getBaseFileName(char* filename, const char* path, u32 tbufSize)
{
    u32 len = (u32)strlen(path);
    assert(len > 0);
    len = min(len, tbufSize - 1);
    for (const char* p = path + len - 1; p != path; --p)
    {
        char c = *p;
        if ('.' == c)
        {
            strncpy(filename, path, p - path);
            filename[p - path] = 0;
            return;
        }
    }
    strncpy(filename, path, len + 1);
}









const char* fileu_filenameExt(const char* filename)
{
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename)
    {
        return "";
    }
    return dot + 1;
}









int fileu_fwrite(FILE* f, const void* buf, u32 size)
{
    return (int)fwrite(buf, 1, size, f);
}

int fileu_fread(FILE* f, void* buf, u32 size)
{
    return (int)fread(buf, 1, size, f);
}











u32 fileu_fileSize(FILE* f)
{
    u32 pos = ftell(f);
    fseek(f, 0, SEEK_END);
    u32 end = ftell(f);
    fseek(f, pos, SEEK_SET);
    return end;
}

u32 fileu_readFile(const char* path, char** buf)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        return -1;
    }
    u32 size = fileu_fileSize(f);
    if (-1 == size)
    {
        return -1;
    }
    if (0 == size)
    {
        return 0;
    }
    *buf = (char*)malloc(size + 1);
    // end c string
    (*buf)[size] = 0;
    size_t r = fread(*buf, 1, size, f);
    if (r != (size_t)size)
    {
        free(*buf);
        *buf = NULL;
        fclose(f);
        return -1;
    }
    fclose(f);
    return size;
}




u32 fileu_writeFile(const char* path, u32 dataSize, const void* data)
{
    FILE* f = fopen(path, "w+b");
    if (!f)
    {
        return -1;
    }
    u32 n = (u32)fwrite(data, 1, dataSize, f);
    if (n != dataSize)
    {
        fclose(f);
        return n;
    }
    fclose(f);
    return n;
}











bool fileu_copyFile(const char* srcPath, const char* dstPath)
{
    int r;
    char* data = NULL;
    r = fileu_readFile(srcPath, &data);
    if (-1 == r)
    {
        return false;
    }
    r = fileu_writeFile(dstPath, r, data);
    if (-1 == r)
    {
        free(data);
        return false;
    }
    free(data);
    return true;
}












bool fileu_fileExist(const char* path)
{
#ifdef _WIN32
    return _access(path, 0) != -1;
#elif defined(__EMSCRIPTEN__)
    FILE* f = fopen(path, "r");
    if (!f)
    {
        return false;
    }
    else
    {
        fclose(f);
        return true;
    }
#else
    return access(path, 0) != -1;
#endif
}


bool fileu_dirExist(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}











void fileu_getTmpDir(char* path)
{
#ifdef _WIN32
    GetTempPathA(fileu_PATH_BUF_MAX, path);
#endif
}

































































































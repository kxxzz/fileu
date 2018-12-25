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


#ifndef _WIN32
# include <dirent.h>
#endif

#ifdef __ANDROID__
# include <unistd.h>
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




static char* stzncpy(char* dst, char const* src, size_t len)
{
    assert(len > 0);
    char* p = memccpy(dst, src, 0, len - 1);
    if (p) --p;
    else
    {
        p = dst + len - 1;
        *p = 0;
    }
    return p;
}






void fileu_getDirName(char* dir, const char* path, u32 tbufSize)
{
    u32 len = (u32)strlen(path);
    len = min(len, tbufSize - 1);
    stzncpy(dir, path, len + 1);
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
            stzncpy(filename, p + 1, len + 1 - (p - path));
            return;
        }
    }
    stzncpy(filename, path, len + 1);
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
            stzncpy(filename, path, p + 1 - path);
            filename[p - path] = 0;
            return;
        }
    }
    stzncpy(filename, path, len + 1);
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


















typedef struct fileu_Dir
{
#ifdef _WIN32
    WIN32_FIND_DATAA fdata;
    HANDLE h;
#else
    DIR* h;
#endif
} fileu_Dir;



fileu_Dir* fileu_openDir(const char* path)
{
#ifdef _WIN32
    char dirPath[fileu_PATH_BUF_MAX] = "";
    stzncpy(dirPath, path, fileu_PATH_BUF_MAX);
    u32 n = strnlen(path, fileu_PATH_BUF_MAX);
    stzncpy(dirPath + n, "/*", fileu_PATH_BUF_MAX - n);
    WIN32_FIND_DATAA fdata;
    HANDLE h = FindFirstFileA(dirPath, &fdata);
    if (INVALID_HANDLE_VALUE == h)
    {
        return NULL;
    }
    fileu_Dir* dir = malloc(sizeof(*dir));
    dir->fdata = fdata;
    dir->h = h;
    return dir;
#else
    DIR* h = opendir(path);
    if (!h)
    {
        return NULL;
    }
    fileu_Dir* dir = malloc(sizeof(*dir));
    dir->h = h;
    return dir;
#endif
}


void fileu_dirClose(fileu_Dir* dir)
{
#ifdef _WIN32
    FindClose(dir->h);
#else
    closedir(dir->h);
#endif
    free(dir);
}


bool fileu_dirNextFile(fileu_Dir* dir, char* file)
{
#ifdef _WIN32
    for (;;)
    {
        if (!FindNextFileA(dir->h, &dir->fdata))
        {
            return false;
        }
        if ((dir->fdata.cFileName[0] != '.') ||
            (dir->fdata.cFileName[1] && (dir->fdata.cFileName[1] != '.')))
        {
            break;
        }
    }
    stzncpy(file, dir->fdata.cFileName, fileu_PATH_BUF_MAX);
    return true;
#else
    struct dirent* entry = NULL;
    for (;;)
    {
        entry = readdir(dir->h);
        if (!entry)
        {
            return false;
        }
        if ((entry->d_name[0] != '.') ||
            (entry->d_name[1] && (entry->d_name[1] != '.')))
        {
            break;
        }
    }
    stzncpy(file, entry->d_name, fileu_PATH_BUF_MAX);
    return true;
#endif
}

































































































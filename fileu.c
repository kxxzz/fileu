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
#ifdef _WIN32
    char* p = _memccpy(dst, src, 0, len - 1);
#else
    char* p = memccpy(dst, src, 0, len - 1);
#endif
    if (p)
    {
        --p;
    }
    else
    {
        p = dst + len - 1;
        *p = 0;
    }
    return p;
}






void FILEU_getDirName(char* dir, const char* path)
{
    u32 len = (u32)strlen(path);
    len = min(len, FILEU_PATH_BUF_MAX - 1);
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

void FILEU_getLocalFileName(char* filename, const char* path)
{
    u32 len = (u32)strlen(path);
    len = min(len, FILEU_PATH_BUF_MAX - 1);
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

void FILEU_getBaseFileName(char* filename, const char* path)
{
    u32 len = (u32)strlen(path);
    assert(len > 0);
    len = min(len, FILEU_PATH_BUF_MAX - 1);
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









const char* FILEU_filenameExt(const char* filename)
{
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename)
    {
        return "";
    }
    return dot + 1;
}









int FILEU_fwrite(FILE* f, const void* buf, u32 size)
{
    return (int)fwrite(buf, 1, size, f);
}

int FILEU_fread(FILE* f, void* buf, u32 size)
{
    return (int)fread(buf, 1, size, f);
}











u32 FILEU_fileSize(FILE* f)
{
    u32 pos = ftell(f);
    fseek(f, 0, SEEK_END);
    u32 end = ftell(f);
    fseek(f, pos, SEEK_SET);
    return end;
}

u32 FILEU_readFile(const char* path, char* buf, u32 bufSize)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        return -1;
    }
    u32 size = FILEU_fileSize(f);
    if (-1 == size)
    {
        fclose(f);
        return -1;
    }
    if (!buf || !bufSize || (bufSize < size))
    {
        fclose(f);
        return size;
    }
    if (size > 0)
    {
        u32 r = (u32)fread(buf, 1, size, f);
        if (r != size)
        {
            fclose(f);
            return r;
        }
    }
    fclose(f);
    return size;
}




u32 FILEU_writeFile(const char* path, const void* data, u32 dataSize)
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











bool FILEU_copyFile(const char* srcPath, const char* dstPath)
{
    int n;
    char* data = NULL;
    n = FILEU_readFile(srcPath, NULL, 0);
    if (-1 == n)
    {
        return false;
    }
    data = malloc(n);
    n = FILEU_readFile(srcPath, data, n);
    if (-1 == n)
    {
        free(data);
        return false;
    }
    n = FILEU_writeFile(dstPath, data, n);
    if (-1 == n)
    {
        free(data);
        return false;
    }
    free(data);
    return true;
}

















const char* FILEU_fileMmap(const char* filename, size_t* pLen)
{
#ifdef _WIN32
    HANDLE file = CreateFileA
    (
        filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL
    );
    if (file == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        fprintf(stderr, "CreateFileA failed, error=%d", err);
        return NULL;
    }
    HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (fileMapping == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        CloseHandle(file);
        fprintf(stderr, "CreateFileMapping failed, error=%d", err);
        return NULL;
    }
    LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
    if (!fileMapView)
    {
        DWORD err = GetLastError();
        CloseHandle(fileMapping);
        CloseHandle(file);
        fprintf(stderr, "MapViewOfFile failed, error=%d", err);
        return NULL;
    }
    return fileMapView;
#else
    FILE* f;
    size_t fileSize;
    struct stat sb;
    char* p;
    int fd;

    f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    fclose(f);

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return NULL;
    }
    if (fstat(fd, &sb) == -1)
    {
        perror("fstat");
        return NULL;
    }
    if (!S_ISREG(sb.st_mode))
    {
        fprintf(stderr, "%s is not a file\n", "lineitem.tbl");
        return NULL;
    }
    p = (char*)mmap(0, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }
    if (close(fd) == -1)
    {
        perror("close");
        return NULL;
    }
    (*pLen) = (size_t)fileSize;
    return p;
#endif
}




void FILEU_fileUnmmap(const char* p)
{
#ifdef _WIN32
    UnmapViewOfFile(p);
    // CloseHandle(fileMapping);
    // CloseHandle(file);
#else
    
#endif
}

















bool FILEU_fileExist(const char* path)
{
#ifdef _WIN32
    struct stat buffer;
    return 0 == stat(path, &buffer);
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


bool FILEU_dirExist(const char* path)
{
    struct stat st[1];
    return (stat(path, st) == 0 && S_ISDIR(st->st_mode));
}
































typedef struct FILEU_Dir
{
#ifdef _WIN32
    WIN32_FIND_DATAA fdata[1];
    HANDLE h;
#else
    DIR* h;
#endif
} FILEU_Dir;



FILEU_Dir* FILEU_openDir(const char* path)
{
#ifdef _WIN32
    char dirPath[FILEU_PATH_BUF_MAX] = "";
    stzncpy(dirPath, path, FILEU_PATH_BUF_MAX);
    u32 n = (u32)strnlen(path, FILEU_PATH_BUF_MAX);
    stzncpy(dirPath + n, "/*", FILEU_PATH_BUF_MAX - n);
    WIN32_FIND_DATAA fdata[1];
    HANDLE h = FindFirstFileA(dirPath, fdata);
    if (INVALID_HANDLE_VALUE == h)
    {
        return NULL;
    }
    FILEU_Dir* dir = malloc(sizeof(*dir));
    dir->fdata[0] = fdata[0];
    dir->h = h;
    return dir;
#else
    DIR* h = opendir(path);
    if (!h)
    {
        return NULL;
    }
    FILEU_Dir* dir = malloc(sizeof(*dir));
    dir->h = h;
    return dir;
#endif
}


void FILEU_dirClose(FILEU_Dir* dir)
{
#ifdef _WIN32
    FindClose(dir->h);
#else
    closedir(dir->h);
#endif
    free(dir);
}


bool FILEU_dirNextFile(FILEU_Dir* dir, char* file)
{
#ifdef _WIN32
    for (;;)
    {
        if (!FindNextFileA(dir->h, dir->fdata))
        {
            return false;
        }
        if ((dir->fdata->cFileName[0] != '.') ||
            (dir->fdata->cFileName[1] && (dir->fdata->cFileName[1] != '.')))
        {
            break;
        }
    }
    stzncpy(file, dir->fdata->cFileName, FILEU_PATH_BUF_MAX);
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
    stzncpy(file, entry->d_name, FILEU_PATH_BUF_MAX);
    return true;
#endif
}

































































































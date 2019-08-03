#pragma once



#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef float f32;
typedef double f64;




enum
{
    FILEU_PATH_BUF_MAX = 2048,
};



void FILEU_getDirName(char* dir, const char* path);
void FILEU_getLocalFileName(char* filename, const char* path);
void FILEU_getBaseFileName(char* filename, const char* path);

const char* FILEU_filenameExt(const char* filename);

int FILEU_fwrite(FILE* f, const void* buf, u32 size);
int FILEU_fread(FILE* f, void* buf, u32 size);

u32 FILEU_fileSize(FILE* f);
u32 FILEU_readFile(const char* path, char* buf, u32 bufSize);
u32 FILEU_writeFile(const char* path, const void* data, u32 dataSize);
bool FILEU_copyFile(const char* srcPath, const char* dstPath);

bool FILEU_fileExist(const char* path);
bool FILEU_dirExist(const char* path);



typedef struct FILEU_Dir FILEU_Dir;

FILEU_Dir* FILEU_openDir(const char* path);
void FILEU_dirClose(FILEU_Dir* dir);
bool FILEU_dirNextFile(FILEU_Dir* dir, char* file);








































































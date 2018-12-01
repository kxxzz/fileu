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
    fileu_PATH_BUF_MAX = 260,
};




void fileu_getDirName(char* dir, const char* path, u32 tbufSize);
void fileu_getLocalFileName(char* filename, const char* path, u32 tbufSize);
void fileu_getBaseFileName(char* filename, const char* path, u32 tbufSize);

const char* fileu_filenameExt(const char* filename);

int fileu_fwrite(FILE* f, const void* buf, u32 size);
int fileu_fread(FILE* f, void* buf, u32 size);

u32 fileu_fileSize(FILE* f);
u32 fileu_readFile(const char* path, char** buf);
u32 fileu_writeFile(const char* path, u32 dataSize, const void* data);
bool fileu_copyFile(const char* srcPath, const char* dstPath);

bool fileu_fileExist(const char* path);
bool fileu_dirExist(const char* path);

void fileu_getTmpDir(char* path);












































































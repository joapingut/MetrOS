#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
#ifdef _NC_RESTRICT
char *strcpy(char *restrict dest, const char *restrict src);
#else
char *strcpy(char *dest, const char* src);
#endif
int strcmp(const char * s1, const char * s2);
int strncmp(const char * s1, const char * s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif

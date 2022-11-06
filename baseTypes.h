#pragma once

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

//#include <assert.h>
/* jeśli te wartości poniżej nie są przestrzegane, */
/* dostaniemy błąd podczas kompilacji */
static_assert(sizeof(s8) == 1, "incorrect size");
static_assert(sizeof(u8) == 1, "incorrect size");
static_assert(sizeof(s16) == 2, "incorrect size");
static_assert(sizeof(u16) == 2, "incorrect size");
static_assert(sizeof(s32) == 4, "incorrect size");
static_assert(sizeof(u32) == 4, "incorrect size");
static_assert(sizeof(s64) == 8, "incorrect size");
static_assert(sizeof(u64) == 8, "incorrect size");

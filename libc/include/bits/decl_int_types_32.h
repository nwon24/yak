#ifndef _BITS_DECL_INT_TYPES_32
#define _BITS_DECL_INT_TYPES_32

#if !defined(__DEFINED_PRIVATE_INT_TYPES)

typedef unsigned char __uint8_t;
typedef unsigned short __uint16_t;
typedef unsigned long __uint32_t;
typedef unsigned long long __uint64_t;
typedef signed char __int8_t;
typedef short __int16_t;
typedef long __int32_t;
typedef long long __int64_t;

typedef unsigned char __uint_least8_t;
typedef unsigned short __uint_least16_t;
typedef unsigned long __uint_least32_t;
typedef unsigned long long __uint_least64_t;

typedef signed char __int_least8_t;
typedef short __int_least16_t;
typedef long __int_least32_t;
typedef long long __int_least64_t;

typedef unsigned char __uint_fast8_t;
typedef unsigned short __uint_fast16_t;
typedef unsigned long __uint_fast32_t;
typedef unsigned long long __uint_fast64_t;

typedef signed char __int_fast8_t;
typedef short __int_fast16_t;
typedef long __int_fast32_t;
typedef long long __int_fast64_t;

typedef __int32_t __intptr_t;
typedef __uint32_t __uintptr_t;

typedef __uint64_t __uintmax_t;
typedef __int64_t __intmax_t;

#define __DEFINED_PRIVATE_INT_TYPES

#endif

#endif

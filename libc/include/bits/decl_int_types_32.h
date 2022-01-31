#ifndef _BITS_DECL_INT_TYPES_32
#define _BITS_DECL_INT_TYPES_32

#if !defined(__DEFINED_PRIVATE_INT_TYPES)

#ifdef __NEED_UINT8_T
typedef unsigned char __uint8_t;
#endif
#ifdef __NEED_UINT16_T
typedef unsigned short __uint16_t;
#endif
#ifdef __NEED_UINT32_T
typedef unsigned long __uint32_t;
#endif
#ifdef __NEED_UINT64_T
typedef unsigned long long __uint64_t;
#endif
#ifdef __NEED_INT8_T
typedef signed char __int8_t;
#endif
#ifdef __NEED_INT16_T
typedef short __int16_t;
#endif
#ifdef __NEED_INT32_T
typedef long __int32_t;
#endif
#ifdef __NEED_INT64_T
typedef long long __int64_t;
#endif

#ifdef __NEED_UINT_LEAST8_T
typedef unsigned char __uint_least8_t;
#endif
#ifdef __NEED_UINT_LEAST16_T
typedef unsigned short __uint_least16_t;
#endif
#ifdef __NEED_UINT_LEAST32_T
typedef unsigned long __uint_least32_t;
#endif
#ifdef __NEED_UINT_LEAST64_T
typedef unsigned long long __uint_least64_t;
#endif

#ifdef __NEED_INT_LEAST8_T
typedef signed char __int_least8_t;
#endif
#ifdef __NEED_INT_LEAST16_T
typedef short __int_least16_t;
#endif
#ifdef __NEED_INT_LEAST32_T
typedef long __int_least32_t;
#endif
#ifdef __NEED_INT_LEAST64_T
typedef long long __int_least64_t;
#endif

#ifdef __NEED_UINT_FAST8_T
typedef unsigned char __uint_fast8_t;
#endif
#ifdef __NEED_UINT_FAST16_T
typedef unsigned short __uint_fast16_t;
#endif
#ifdef __NEED_UINT_FAST32_T
typedef unsigned long __uint_fast32_t;
#endif
#ifdef __NEED_UINT_FAST64_T
typedef unsigned long long __uint_fast64_t;
#endif

#ifdef __NEED_INT_FAST8_T
typedef signed char __int_fast8_t;
#endif
#ifdef __NEED_INT_FAST16_T
typedef short __int_fast16_t;
#endif
#ifdef __NEED_INT_FAST32_T
typedef long __int_fast32_t;
#endif
#ifdef __NEED_INT_FAST64_T
typedef long long __int_fast64_t;
#endif

#ifdef __NEED_INTPTR_T
typedef __int32_t __intptr_t;
#endif
#ifdef __NEED_UINTPTR_T
typedef __uint32_t __uintptr_t;
#endif

#ifdef __NEED_UINTMAX_T
typedef __uint64_t __uintmax_t;
#endif
#ifdef __NEED_INTMAX_T
typedef __int64_t __intmax_t;
#endif

#define __DEFINED_PRIVATE_INT_TYPES

#endif

#endif

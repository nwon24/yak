/*
 * stdio.c
 * Internal misc. stuff that is not visible to programs (__*)
 */
#include <stdio.h>

FILE __file_table[FOPEN_MAX];

/*
 * proc.c
 * Handles processes.
 */
#include <kernel/proc.h>

struct process process_table[NR_PROC];

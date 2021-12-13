#ifndef _INIT_H
#define _INIT_H

typedef int (*init_function)(void);

extern init_function init_func_table[];

void run_driver_init_functions(void);

#endif /* _INIT_H */

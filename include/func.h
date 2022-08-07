#ifndef _FUNC_H
#define _FUNC_H

#include <stdbool.h>

void *salloc(int width, int *len, int dim);
bool str_to_num(char *str, int *n);
int *distinct_rand(int start, int end, int count);

#endif

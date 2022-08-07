#include "func.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void *salloc(int width, int *len, int dim) {
    assert(dim > 0 && width > 0);
    int total_len = 0;
    int temp = len[0];
    for (int i = 1; i < dim; i++) {
        total_len += sizeof(void *) * temp;
        temp *= len[i];
    }
    total_len += temp * width;
    void *output = malloc(total_len);
    if (dim == 1) {
        return output;
    }
    void **cur = (void **)output;
    char *target = (char *)&cur[len[0]];
    temp = len[0];
    for (int i = 1; i < dim - 1; i++) {
        for (int j = 0; j < temp; j++) {
            cur[j] = (void *)&target[sizeof(void *) * j * len[i]];
        }
        cur += temp;
        temp *= len[i];
        target += sizeof(void *) * temp;
    }
    for (int i = 0; i < temp; i++) {
        cur[i] = (void *)&target[width * i * len[dim - 1]];
    }
    return output;
}

bool str_to_num(char *str, int *n) {
    int len = strlen(str);
    if (!len) {
        return false;
    }
    int output = 0;
    bool negative = false;
    int i = 0;
    if (str[i] == '-') {
        i++;
        negative = true;
    }
    for (; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
        output *= 10;
        output += str[i] - '0';
    }
    output *= negative ? -1 : 1;
    *n = output;
    return true;
}

int *distinct_rand(int start, int end, int count) {
    int total = end - start + 1;
    assert(total >= count);
    int *cand = (int *)malloc(sizeof(int) * total);
    for (int i = 0; i < total; i++) {
        cand[i] = start + i;
    }
    for (int i = 0; i < count; i++) {
        int r = rand() % (total - i) + i;
        int temp = cand[i];
        cand[i] = cand[r];
        cand[r] = temp;
    }
    int *output = (int *)malloc(sizeof(int) * count);
    memcpy(output, cand, sizeof(int) * count);
    free(cand);
    return output;
}

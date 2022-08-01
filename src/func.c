#include "func.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

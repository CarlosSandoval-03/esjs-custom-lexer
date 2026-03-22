#include <stdio.h>
#include "../include/buffer.h"

int main(void) {
    Buffer buf;
    int c;
    size_t i = 0;

    buffer_init(&buf, stdin);
    while ((c = buffer_get(&buf, i)) != EOF) {
        printf("buf[%zu] = '%c'\n", i, c);
        i++;
    }

    buffer_destroy(&buf);
    return 0;
}
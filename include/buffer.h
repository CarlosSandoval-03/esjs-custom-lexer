#ifndef ESJS_CUSTOM_LEXER_BUFFER_H
#define ESJS_CUSTOM_LEXER_BUFFER_H

#include <stdio.h>

typedef struct {
    FILE *input; // Data origin
    int fd; // Underlying file descriptor
    int nonblocking; // Whether tty input is being drained in batch mode

    char *data; // Stored data
    size_t length; // Number of chars loaded into buffer
    size_t capacity; // Allocated size
    int eof; // Boolean that indicate if the EOF is reached
} Buffer;

void buffer_init(Buffer * buf, FILE * input);

void buffer_destroy(Buffer *buf);

int buffer_get(Buffer *buf, size_t pos);

int buffer_is_eof(const Buffer *buf);

#endif // ESJS_CUSTOM_LEXER_BUFFER_H
#include "../include/buffer.h"

#include <stdlib.h>

#define BUFFER_INITIAL_CAPACITY 128

static void buffer_grow(Buffer *buf) {
  const size_t new_capacity =
      (buf->capacity == 0) ? BUFFER_INITIAL_CAPACITY : buf->capacity * 2;
  char *new_data = (char *)realloc(buf->data, new_capacity * sizeof(char));

  if (new_data == NULL) {
    fprintf(stderr, "Error: The buffer could not be expanded.\n");
    exit(EXIT_FAILURE);
  }

  buf->data = new_data;
  buf->capacity = new_capacity;
}

static void buffer_fill_to(Buffer *buf, const size_t pos) {
  while ((buf->length <= pos) && !buf->eof) {
    const int c = fgetc(buf->input);

    if (c == EOF) {
      buf->eof = 1;
      return;
    }

    if (buf->length == buf->capacity) {
      buffer_grow(buf);
    }

    buf->data[buf->length++] = (char)c;
  }
}

void buffer_init(Buffer *buf, FILE *input) {
  buf->data = NULL;
  buf->length = 0;
  buf->capacity = 0;
  buf->eof = 0;
  buf->input = input;
}

void buffer_destroy(Buffer *buf) {
  free(buf->data);
  buf->data = NULL;
  buf->length = 0;
  buf->capacity = 0;
  buf->eof = 1;
  buf->input = NULL;
}

int buffer_get(Buffer *buf, const size_t pos) {
  buffer_fill_to(buf, pos);

  if (pos >= buf->length) {
    return EOF;
  }

  return (unsigned char)buf->data[pos];
}

/**
 * @file buffer.c
 * @brief Lazily-loaded, dynamically-growing input buffer implementation.
 *
 * The buffer reads bytes from a FILE* stream on demand. Data is never
 * discarded: once a byte has been loaded it stays in the heap array so that
 * the scanner can backtrack by simply re-reading a lower position.
 *
 * Growth strategy:
 *  - Initial capacity: BUFFER_INITIAL_CAPACITY (128 bytes).
 *  - On overflow: capacity doubles. This amortizes the cost of realloc() to
 *    O(1) per byte over the lifetime of the buffer.
 */
#include "../include/buffer.h"

#include <stdlib.h>

/** Initial heap allocation size in bytes for a newly created buffer. */
#define BUFFER_INITIAL_CAPACITY 128

/**
 * @brief Doubles the buffer's heap capacity.
 *
 * Reallocates @c buf->data to a new block of either BUFFER_INITIAL_CAPACITY
 * bytes (when the current capacity is zero) or twice the current capacity.
 * Terminates the process with an error message if the allocation fails, as
 * there is no sensible way to recover from out-of-memory inside a lexer.
 *
 * @param buf Buffer whose data array must be enlarged. Must not be NULL.
 */
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

/**
 * @brief Reads bytes from the stream until position @p pos is covered.
 *
 * Reads one character at a time via fgetc() and appends it to @c buf->data,
 * growing the array when capacity is exhausted. Sets @c buf->eof when the
 * stream returns EOF and stops immediately.
 *
 * @param buf Buffer to fill. Must not be NULL.
 * @param pos Target position that must be within the loaded range after the
 *            call (subject to EOF).
 */
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

/**
 * @brief Initializes a Buffer to an empty, ready-to-use state.
 *
 * No heap memory is allocated here; the first buffer_get() call will trigger
 * the initial load. The @p input stream is stored by pointer and must remain
 * open until buffer_destroy() is called.
 *
 * @param buf   Buffer to initialize. Must not be NULL.
 * @param input Readable FILE stream that provides the input characters.
 */
void buffer_init(Buffer *buf, FILE *input) {
  buf->data = NULL;
  buf->length = 0;
  buf->capacity = 0;
  buf->eof = 0;
  buf->input = input;
}

/**
 * @brief Releases all resources held by the buffer and resets it.
 *
 * Frees the heap-allocated data array and sets all fields to safe defaults.
 * The associated FILE* is not closed here; the caller retains responsibility
 * for closing the stream.
 *
 * @param buf Buffer to destroy. Must not be NULL.
 */
void buffer_destroy(Buffer *buf) {
  free(buf->data);
  buf->data = NULL;
  buf->length = 0;
  buf->capacity = 0;
  buf->eof = 1;
  buf->input = NULL;
}

/**
 * @brief Returns the byte at absolute position @p pos, loading from the
 *        stream if that position has not been read yet.
 *
 * If the position is beyond the end of the stream, EOF is returned and the
 * buffer is not modified. The return type is @c int (not @c char) to match
 * the convention of fgetc() and allow unambiguous EOF detection.
 *
 * @param buf Buffer to read from. Must not be NULL.
 * @param pos Zero-based byte offset into the input stream.
 * @return    The byte at @p pos as an @c unsigned @c char widened to @c int,
 *            or EOF if @p pos is at or beyond end-of-stream.
 */
int buffer_get(Buffer *buf, const size_t pos) {
  buffer_fill_to(buf, pos);

  if (pos >= buf->length) {
    return EOF;
  }

  return (unsigned char)buf->data[pos];
}

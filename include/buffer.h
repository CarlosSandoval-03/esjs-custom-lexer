/**
 * @file buffer.h
 * @brief Lazily-loaded, dynamically-growing input buffer for the lexer.
 *
 * The Buffer abstracts the underlying FILE* stream so that the scanner and
 * DFA can perform arbitrary lookahead and backtrack freely by re-reading
 * already-loaded bytes via buffer_get(). Data is read on demand: bytes are
 * fetched from the stream only when a position beyond the currently loaded
 * range is accessed.
 *
 * Memory management:
 *  - buffer_init()    sets the initial empty state (no heap allocation yet).
 *  - buffer_get()     triggers lazy loading when necessary, growing the
 *                     internal array if capacity is exhausted.
 *  - buffer_destroy() releases the heap-allocated data array and resets all
 *                     fields to safe defaults.
 *
 * The caller is responsible for keeping the FILE* open for as long as the
 * Buffer is in use.
 */
#ifndef ESJS_CUSTOM_LEXER_BUFFER_H
#define ESJS_CUSTOM_LEXER_BUFFER_H

#include <stdio.h>

/**
 * @brief Lazily-loaded byte buffer backed by a FILE stream.
 *
 * All bytes loaded so far are retained in @c data so that the scanner can
 * seek backward (by re-reading lower positions) without re-opening the
 * stream. This enables unlimited lookahead and trivial backtracking.
 *
 * @warning **Not thread-safe.** A Buffer must not be shared between threads.
 *          Concurrent calls to buffer_get() on the same instance race on
 *          `length`, `capacity`, and `data`: a realloc() triggered by one
 *          thread moves the heap block, leaving any pointer cached by another
 *          thread (including Token::lexeme_start values) dangling. Each thread
 *          must own its own Buffer backed by its own FILE* stream.
 */
typedef struct {
  FILE   *input;    /**< Source stream. Read sequentially by buffer_fill_to(). */
  char   *data;     /**< Heap-allocated array of loaded bytes. May be NULL
                         before the first call to buffer_get().               */
  size_t  length;   /**< Number of bytes currently loaded into @c data.       */
  size_t  capacity; /**< Allocated size of @c data in bytes.                  */
  int     eof;      /**< Non-zero once the underlying stream returns EOF.      */
} Buffer;

/**
 * @brief Initializes a Buffer to an empty state backed by @p input.
 *
 * No heap memory is allocated and no bytes are read at this point. The first
 * call to buffer_get() will trigger the initial load.
 *
 * @param buf   Buffer instance to initialize. Must not be NULL.
 * @param input Open readable FILE stream to read from. Must remain open
 *              until buffer_destroy() is called.
 */
void buffer_init(Buffer *buf, FILE *input);

/**
 * @brief Releases all heap memory and resets the Buffer to an inert state.
 *
 * After this call @p buf must not be used again without a subsequent
 * buffer_init(). The caller is responsible for closing the FILE* separately.
 *
 * @param buf Buffer instance to destroy. Must not be NULL.
 */
void buffer_destroy(Buffer *buf);

/**
 * @brief Returns the byte at absolute position @p pos, loading from the
 *        stream if necessary.
 *
 * If @p pos is within the already-loaded range the byte is returned
 * immediately. Otherwise the buffer reads ahead until the position is
 * covered, growing the internal array as needed. Returns EOF when @p pos
 * is beyond the end of the input stream.
 *
 * @param buf Buffer instance. Must not be NULL.
 * @param pos Zero-based byte offset into the input stream.
 * @return    The byte at @p pos cast to @c unsigned @c char and widened to
 *            @c int, or EOF if the position is past end-of-stream.
 */
int buffer_get(Buffer *buf, size_t pos);

#endif  // ESJS_CUSTOM_LEXER_BUFFER_H
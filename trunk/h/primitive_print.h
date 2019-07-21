#ifndef PRIMITIVE_PRINT_H
#define PRIMITIVE_PRINT_H

#include <stdint.h>

// primitive_print.h -- Defines the interface for primitive_print.h
//              From JamesM's kernel development tutorials.


// Clear the screen to all black.
void primitive_clear();
int primitive_printf(const char *s, ...);

#endif /* PRIMITIVE_PRINT_H */

#include <stdio.h>
#include <stdarg.h>

void print_error(const char * format, ...) {
  va_list vl;
  va_start(vl, format);
  vprintf(format, vl);
  va_end(vl);
}

#ifndef SLOW5_SUPPORT_H
#define SLOW5_SUPPORT

#include <string>
#include "reads.h"
#include <slow5/slow5.h>

void slow5_print_version();
void slow5_getSignal(DNAscent::read &r, slow5_file_t *sp);
#endif

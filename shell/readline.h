#ifndef READLINE_H
#define READLINE_H
#include "history.h"
#define CHAR_EOF '\004'
#define CHAR_DEL 127
#define CHAR_ESC '\033'
#define CHAR_BYTE_2 "\r\x1b[2C"
#define CHAR_EMPTY_LINE "\033[K"


char *read_line(const char *prompt, struct history *hist, int event);
#endif  // READLINE_H

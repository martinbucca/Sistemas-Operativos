#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"
#include "history.h"


extern char prompt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

int history(char *cmd, const struct history *hist);

#endif  // BUILTIN_H

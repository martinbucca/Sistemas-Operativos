#ifndef RUNCMD_H
#define RUNCMD_H

#include "defs.h"
#include "parsing.h"
#include "exec.h"
#include "printstatus.h"
#include "freecmd.h"
#include "builtin.h"
#include "history.h"


int run_cmd(char *cmd, const struct history *hist);
#endif  // RUNCMD_H

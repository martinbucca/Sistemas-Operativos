#ifndef SHELL_HISTORY_H
#define SHELL_HISTORY_H

#include <stdbool.h>

struct node {
	char *command;
	struct node *next;
	struct node *prev;
};

struct history {
	struct node *first;
	struct node *last;
	struct node *current_at_save;
	struct node *current_at_move;
};

struct history *init_history(void);
const char *get_root_history(void);
void add_command(struct history *hist, const char *command);
bool cargar_comandos(struct history *hist);
void destroy_history(struct history *hist);
const char *move_up(struct history *hist);
const char *move_down(struct history *hist);
void load_history_from_file(struct history *hist);
#endif  // SHELL_HISTORY_H

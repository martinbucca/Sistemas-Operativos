#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "defs.h"

struct history *
init_history()
{
	struct history *hist = malloc(sizeof(struct history));
	if (hist == NULL) {
		perror("Error, no se pudo inicializar el historial");
		return NULL;
	}
	hist->first = NULL;
	hist->last = NULL;
	hist->current_at_move = NULL;
	load_history_from_file(hist);
	return hist;
}


bool
cargar_comandos(struct history *hist)
{
	if (hist == NULL) {
		return false;
	}
	struct node *aux;
	if (hist->current_at_save == NULL) {
		aux = hist->first;
	} else if (hist->current_at_save->next == NULL) {
		aux = NULL;
	} else {
		aux = hist->current_at_save->next;
	}
	const char *path = get_root_history();
	FILE *new_file = fopen(path, "a");
	if (new_file == NULL) {
		perror("Error opening file");
		return false;
	}
	while (aux != NULL) {
		fprintf(new_file, "%s\n", aux->command);
		aux = aux->next;
	}
	fclose(new_file);
	return true;
}
void
add_command(struct history *hist, const char *command)
{
	if (hist == NULL) {
		return;
	}
	struct node *new_node = malloc(sizeof(struct node));
	if (new_node == NULL) {
		perror("Error, no se pudo agregar el comando al historial");
		return;
	}
	new_node->command = malloc(strlen(command) + 1);
	if (new_node->command == NULL) {
		perror("Error, no se pudo agregar el comando al historial");
		return;
	}
	strcpy(new_node->command, command);
	if (hist->first == NULL) {
		new_node->next = NULL;
		new_node->prev = NULL;
		hist->first = new_node;
		hist->last = new_node;
	} else {
		new_node->next = NULL;
		hist->last->next = new_node;
		new_node->prev = hist->last;
		hist->last = new_node;
	}
	// Siempre es el ultimo comando ingresado
	hist->current_at_move = new_node;
}
const char *
get_root_history()
{
	char *env_history = getenv("HISTFILE");
	if (env_history == NULL) {
		char *home_temp = getenv("HOME");
		char home_copy[BUFLEN];
		strcpy(home_copy, home_temp);
		char *path = strcat(home_copy, "/.fisop_history");
		// si no existe lo crea
		FILE *new_file = fopen(path, "a");
		if (new_file == NULL) {
			perror("Error opening file");
		}
		fclose(new_file);
		return path;
	}
	return env_history;
}
void
destroy_history(struct history *hist)
{
	struct node *aux = hist->first;
	while (aux != NULL) {
		struct node *next = aux->next;
		free(aux->command);
		free(aux);
		aux = next;
	}
	free(hist);
}

const char *
move_up(struct history *hist)
{
	if (hist == NULL || hist->current_at_move == NULL) {
		return NULL;
	}
	const char *command = hist->current_at_move->command;
	if (hist->current_at_move->prev != NULL) {
		hist->current_at_move = hist->current_at_move->prev;
	}
	return command;
}

const char *
move_down(struct history *hist)
{
	if (hist == NULL || hist->current_at_move == NULL ||
	    hist->current_at_move->next == NULL) {
		return NULL;
	}
	// me aseguro de que no es NULL
	const char *command = hist->current_at_move->next->command;
	hist->current_at_move = hist->current_at_move->next;
	return command;
}

void
load_history_from_file(struct history *hist)
{
	const char *env_history = get_root_history();
	FILE *file = fopen(env_history, "r");
	if (file == NULL) {
		perror("Error, no se pudo abrir el archivo");
		return;
	}
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, file)) != -1) {
		line[strcspn(line, "\n")] = '\0';
		add_command(hist, line);
	}
	/*Apunta al ultimo nodo para al momento de escribir en el archivo
	 * empieze a escribir a partir desde este ultimo. Puede ser que sea
	 * NULL ya que el archivo esta vacio. En ese caso se escribe la lista
	 * completa de comandos.*/
	if (hist->last != NULL) {
		hist->current_at_save = hist->last;
	} else {
		hist->current_at_save = NULL;
	}
	free(line);
	fclose(file);
}
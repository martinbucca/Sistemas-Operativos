#include "defs.h"
#include "readline.h"
#include <termios.h>
#include <ctype.h>
#include "utils.h"

static char buffer[BUFLEN];
struct termios saved_attributes;

static void
set_input_mode(void)
{
	struct termios tattr;

	/* Make sure stdin is a terminal. */
	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Not a terminal.\n");
		exit(EXIT_FAILURE);
	}

	/* Save the terminal attributes so we can restore them later. */
	tcgetattr(STDIN_FILENO, &saved_attributes);

	/* Set the funny terminal modes. */
	tcgetattr(STDIN_FILENO, &tattr);
	/* Clear ICANON and ECHO. We'll do a manual echo! */
	tattr.c_lflag &= ~(ICANON | ECHO);
	/* Read one char at a time */
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}
static void
reset_input_mode(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

static char *
read_canonical_mode()
{
	int i = 0, c = 0;
	memset(buffer, 0, BUFLEN);

	c = getchar();

	while (c != END_LINE && c != EOF) {
		buffer[i++] = c;
		c = getchar();
	}

	// if the user press ctrl+D
	// just exit normally
	if (c == EOF)
		return NULL;

	buffer[i] = END_STRING;

	return buffer;
}

static char *
read_noncanonical_mode(struct history *hist, int event)
{
	int i = 0;
	memset(buffer, 0, BUFLEN);
	bool come_from_esc = false;
	// event designator !!
	if (event == RUN_EVENT_DESIGNATOR_1) {
		if (hist->last != NULL) {
			strcpy(buffer, hist->last->command);
		}
		write(STDOUT_FILENO, buffer, sizeof buffer);
		come_from_esc = true;
	} else if (event > RUN_EVENT_DESIGNATOR_1) {  // case !-n
		int i = 0;
		struct node *aux = hist->last;
		while (i < event && aux != NULL) {
			aux = aux->prev;
			i++;
		}
		if (aux != NULL) {
			strcpy(buffer, aux->command);
		}
		write(STDOUT_FILENO, buffer, sizeof buffer);
		come_from_esc = true;
	}
	char c;
	while (1) {
		// MODO NO CANONICO
		set_input_mode();
		read(STDIN_FILENO, &c, 1);  // leo el caracter y lo guardo en c
		if (c == CHAR_DEL) {
			// si esta al inicio de la linea no hace nada
			if (i == 0) {
				continue;
			}
			// borra caracter
			write(STDOUT_FILENO, "\b \b", 3);
			buffer[i--] = '\0';
		} else if (c == CHAR_EOF) {  // Ctrl-D
			// vuelvo a modo canonico
			reset_input_mode();
			return NULL;
		} else if (c == END_LINE) {
			// escribo el \n y rompo para que devuelva el commando leido hasta el \n
			write(STDOUT_FILENO, &c, 1);
			break;
		} else if (c == CHAR_ESC) {
			read(STDIN_FILENO, &c, 1);
			read(STDIN_FILENO, &c, 1);
			const char *command;
			switch (c) {
			case UP:
				command = move_up(hist);
				if (command == NULL) {
					break;
				}
				write(STDOUT_FILENO,
				      CHAR_BYTE_2,
				      strlen(CHAR_BYTE_2));  // mover el cursor al inicio de la línea + 2
				write(STDOUT_FILENO,
				      CHAR_EMPTY_LINE,
				      strlen(CHAR_EMPTY_LINE));  // borra todo desde donde esta el cursor
				memset(buffer, '\0', BUFLEN);
				strcpy(buffer, command);
				write(STDOUT_FILENO, buffer, sizeof buffer);
				come_from_esc = true;
				break;
			case DOWN:
				command = move_down(hist);
				if (command == NULL) {
					break;
				}
				write(STDOUT_FILENO,
				      CHAR_BYTE_2,
				      strlen(CHAR_BYTE_2));  // mover el cursor al inicio de la línea + 2
				write(STDOUT_FILENO,
				      CHAR_EMPTY_LINE,
				      strlen(CHAR_EMPTY_LINE));  // borra todo desde donde esta el cursor
				memset(buffer, '\0', BUFLEN);
				strcpy(buffer, command);
				write(STDOUT_FILENO, buffer, sizeof buffer);
				come_from_esc = true;
				break;
			default:
				break;
			}
		} else if (isprint(c)) {
			write(STDOUT_FILENO, &c, 1);
			buffer[i++] = c;
		}
		reset_input_mode();
	}
	if (!come_from_esc) {
		buffer[i] = END_STRING;
	}
	// vuelvo al modo canonico
	reset_input_mode();
	return buffer;
}
// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt, struct history *hist, int event)
{
#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
	fprintf(stdout, "%s", "$ ");
	fflush(stdout);
#endif
	if (!isatty(STDIN_FILENO)) {
		return read_canonical_mode();
	} else {
		return read_noncanonical_mode(hist, event);
	}
}

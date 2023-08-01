#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include "history.h"
#include <ctype.h>


char prompt[PRMTLEN] = { 0 };

// runs a shell command
static void
run_shell()
{
	struct history *hist =
	        NULL;  // Es necesario agregarla para que corran las pruebas
	               // sino en readline no se le puede pasar por parametro.
#ifndef SHELL_NO_INTERACTIVE
	hist = init_history();
#endif

	char *cmd;

	while ((cmd = read_line(prompt, hist, NOT_EVENT_DESIGNATOR)) != NULL) {
#ifndef SHELL_NO_INTERACTIVE
		// case cmd = !!
		if (strcmp(cmd, SIMPLE_EVENT_DESIGNATOR) == 0) {
			cmd = read_line(prompt, hist, RUN_EVENT_DESIGNATOR_1);
		} else if (strlen(cmd) > 2 && cmd[0] == '!' && cmd[1] == '-' &&
		           isdigit(cmd[2])) {
			char *numero = split_line(cmd, '-');
			int i = 0;
			while (numero[i] != '\0') {
				if (isdigit(numero[i]) == 0) {
					printf_debug("No es un numero\n");
					break;
					;
				}
				i++;
			}
			int num = atoi(numero);
			if (num < 1) {
				printf_debug("ERROR el numero debe ser mayor o "
				             "igual a 1\n");
			} else {
				cmd = read_line(prompt, hist, num);
			}
		}
		add_command(hist, cmd);
#endif
		if (run_cmd(cmd, hist) == EXIT_SHELL) {
#ifndef SHELL_NO_INTERACTIVE
			cargar_comandos(hist);  // Escribe en el archivo al final
			destroy_history(hist);
#endif
			return;
		}
	}
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}

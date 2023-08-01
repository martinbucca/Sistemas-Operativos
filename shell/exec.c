#include "exec.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	for (int i = 0; i < eargc; i++) {
		char *actual_argument = eargv[i];
		char key[BUFLEN];
		get_environ_key(actual_argument, key);
		char value[BUFLEN];
		int index_where_equals_resides =
		        block_contains(actual_argument, '=');
		if (index_where_equals_resides < 0) {
			printf_debug("Error, argumento invalido");
			exit(-1);
		}
		get_environ_value(actual_argument,
		                  value,
		                  index_where_equals_resides);
		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	// Your code here
	if (flags & O_CREAT) {
		return open(file, flags, S_IRUSR | S_IWUSR);
	}
	return open(file, flags);
}

// Opens the file passed as parameter with the aproppiate flags and
// redirects the flow of the new_file into the file
static void
redirect_flow(char *file, int new_file, int flags)
{
	int fd = open_redir_fd(file, flags);
	if (fd < 0) {
		printf_debug("Error, no se pudo abrir el archivo\n");
		exit(-1);
	}
	int d = dup2(fd, new_file);
	if (d < 0) {
		printf_debug("Error en dup2");
		exit(-1);
	}
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;
	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		// Your code here
		e = (struct execcmd *) cmd;  // casteo el comando a execcmd
		set_environ_vars(e->eargv,
		                 e->eargc);  // inicializo variables de entorno
		if (execvp(e->argv[0], e->argv) < 0) {
			printf_debug("No se pudo ejecutar el comando\n");
			exit(-1);
		}
		break;

	case BACK: {
		// runs a command in background
		//
		// Your code here
		b = (struct backcmd *) cmd;  // casteo el comando a backcmd
		int f = fork();
		if (f < 0) {
			printf_debug("Error en fork");
			exit(-1);
		}
		if (f == 0) {
			exec_cmd(b->c);  // si estoy en el hijo ejecuto el background command
		}
		// si estoy en el padre espero al proceso hijo para que no quede
		// huerfano, pero le agrego el flag WNOHANG para que el proceso
		// padre no quede bloqueado y siga ejecutandose mientras espera.
		int status;
		waitpid(f, &status, WNOHANG);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;  // casteo al comando execcmd
		if (strlen(r->out_file)) {
			// direcciono el stdout al archivo guardado en el campo r->out_file
			redirect_flow(r->out_file,
			              1,
			              O_CLOEXEC | O_CREAT | O_TRUNC | O_WRONLY);
		}
		if (strlen(r->in_file)) {
			// direcciono el stdin al archivo guardado en el campo r->in_file
			redirect_flow(r->in_file, 0, O_CLOEXEC | O_RDONLY);
		}
		if (strlen(r->err_file)) {
			if (strlen(r->err_file) == 2 && r->err_file[0] == '&' &&
			    r->err_file[1] == '1') {
				// direcciono la salida por stderr al stdout (el
				// stdout va a estar apuntando al r->file_out)
				int d = dup2(1, 2);
				if (d < 0) {
					printf_debug("Error en dup2");
					exit(-1);
				}
			} else {
				// direcciono el stderr al archivo guardado en el campo r->err_file
				redirect_flow(r->err_file,
				              2,
				              O_CLOEXEC | O_CREAT | O_TRUNC |
				                      O_WRONLY);
			}
		}
		cmd->type = EXEC;
		exec_cmd(cmd);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		p = (struct pipecmd *) cmd;  // casteo al comando pipecmd
		int fds[2];
		int pip = pipe(fds);
		if (pip < 0) {
			printf_debug("Error en pipe");
			exit(-1);
		}
		int izq = fork();
		if (izq < 0) {
			printf_debug("Error en fork");
			exit(-1);
		}
		if (izq == 0) {
			int left_dup = dup2(fds[WRITE], 1);
			close(fds[WRITE]);
			close(fds[READ]);
			if (left_dup < 0) {
				printf_debug("Error en dup2");
				exit(-1);
			}
			exec_cmd(p->leftcmd);
			exit(-1);
		}
		int der = fork();
		if (der < 0) {
			printf_debug("Error en fork");
			exit(-1);
		}
		if (der == 0) {
			int rigth_dup = dup2(fds[READ], 0);
			close(fds[READ]);
			close(fds[WRITE]);
			if (rigth_dup < 0) {
				printf_debug("Error en dup2");
				exit(-1);
			}
			exec_cmd(p->rightcmd);
			exit(-1);
		}
		close(fds[WRITE]);
		close(fds[READ]);
		int status;
		waitpid(izq, &status, 0);
		waitpid(der, &status, 0);

		// free the memory allocated
		// for the pipe tree structure
		// free_command(parsed_pipe);

		break;
	}
	}
}

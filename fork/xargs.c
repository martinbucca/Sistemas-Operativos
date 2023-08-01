#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

static int
validar_parametros(int argc)
{
	if (argc != 2) {
		return 0;
	}
	return 1;
}

static void
liberar_argumentos_leidos(char **args, int cantidad_de_argumentos)
{
	// el primer elemento es el comando y no se debe liberar y el ultimo es
	// null y tampoco se debe liberar se liberan desde la posicion 1 hasta
	// la posicion cantidad de elemntos + 1 --> son todos los argumentos
	// leidos
	for (int i = 1; i < (cantidad_de_argumentos + 2); i++) {
		free(args[i]);
		args[i] = NULL;
	}
}

static void
ejecutar(char *args[])
{
	int f = fork();
	if (f < 0) {
		fprintf(stderr, "Error en fork! %d\n", f);
		exit(-1);
	}
	if (f == 0) {
		// si estoy en el proceso hijo, ejecuto los argumentos
		execvp(args[0], args);
	} else {
		// si estoy en el proceso padre, espero a que termine el proceso hijo, para que no quede zombie
		wait(NULL);
	}
}

static void
leer_argumentos(char **args)
{
	int cant_argumentos = 0;
	// si len = 0 --> getline() asigna automaticamente memoria para el lugar donde se guarda lo leido
	size_t len = 0;
	while (getline(&args[cant_argumentos + 1], &len, stdin) != -1) {
		cant_argumentos++;
		// le saco el "\n" a la linea leida
		args[cant_argumentos][strcspn(args[cant_argumentos], "\n")] = '\0';
		if (cant_argumentos == NARGS) {
			ejecutar(args);
			liberar_argumentos_leidos(args, cant_argumentos);
			cant_argumentos = 0;
		}
	}
	// si getline falla, es decir no lee mas. igual se hace un alloc y se
	// debe liberar ese valor y asignar a NULL porque getline asigna
	// automaticamente memoria para ese lugar
	free(args[cant_argumentos + 1]);
	args[cant_argumentos + 1] = NULL;
	if (cant_argumentos > 0) {
		// si quedaron argumentos y no llegan a ser NARGS se ejecutan esos que quedan
		ejecutar(args);
		liberar_argumentos_leidos(args, cant_argumentos);
	}
}


int
main(int argc, char *argv[])
{
	if (!validar_parametros(argc)) {
		perror("Parametros invalidos");
		return 1;
	}
	char *cmd = argv[1];
	// el tamaño maximo va a ser NARGS + 1 (porque el primer elemento es el
	// cmd) + 1 (ultimo elemento siempre va a ser NULL)
	char *args[NARGS + 2] = { NULL };
	args[0] = cmd;
	leer_argumentos(args);
	return 0;
}

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static int
es_numero_valido(char *cadena)
{
	while (*cadena != '\0') {
		if (!isdigit(*cadena)) {
			return 0;
		}
		cadena += 1;
	}
	// todos los digitos son enteros --> numero entero
	return 1;
}

static int
validar_parametros(int argc, char *argv[])
{
	if (argc != 2 || !es_numero_valido(argv[1]) || atoi(argv[1]) < 2) {
		return 0;
	}
	return 1;
}


static void
filtrar(int fds[])
{
	close(fds[1]);
	int p = 0;
	// leo el primer valor que es primo
	if (read(fds[0], &p, sizeof(p)) > 0) {
		printf("primo %d\n", p);
	} else {
		close(fds[0]);
		exit(-1);
		return;
	}
	int fds_h[2];
	// creo pipe para el hijo
	int p_hijo = pipe(fds_h);
	if (p_hijo < 0) {
		perror("Error en segundo pipe");
		exit(-1);
	}
	int n = 0;
	while (read(fds[0], &n, sizeof(p)) > 0) {
		if (n % p != 0) {
			int envio = write(fds_h[1], &n, sizeof(n));
			if (envio < 0) {
				perror("Error en write! \n");
				exit(-1);
			}
		}
	}
	// ya termine de escribir en el pipe del hijo, lo cierro.
	close(fds_h[1]);
	// cierro el pipe de lectura del padre
	close(fds[0]);
	// creo proceso hijo
	int f = fork();
	if (f == 0) {
		filtrar(fds_h);
		close(fds_h[0]);
		exit(-1);
		return;
	} else {
		wait(NULL);
		exit(-1);
	}
	return;
}

static void
generador(int n)
{
	int fds[2];
	// creo el primer pipe
	int p = pipe(fds);
	if (p < 0) {
		perror("Error en segundo pipe");
		exit(-1);
	}
	// creo el primer proceso hijo
	int f = fork();
	if (f == 0) {
		// estoy en el primer proceso hijo filtro
		filtrar(fds);
		// close(fds[0]);
		exit(-1);
	} else {
		// si estoy en el primer proceso padre genero secuencia de numeros y los escribo en el pipe
		for (int i = 2; i <= n; i++) {
			int envio = write(fds[1], &i, sizeof(i));
			if (envio < 0) {
				perror("Error en write! \n");
				exit(-1);
			}
		}
		// cierro el fd de escritura en el proceso padre
		close(fds[1]);
		// espero que el hijo termine para que no quede zombie
		wait(NULL);
		close(fds[0]);
	}
}


int
main(int argc, char *argv[])
{
	if (!validar_parametros(argc, argv)) {
		perror("Parametros invalidos");
		return 1;
	}
	int n = atoi(argv[1]);
	generador(n);
	return 0;
}
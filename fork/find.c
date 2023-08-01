#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

#define MAX_PATH 1600
typedef char *(*buscar_cadena)(const char *, const char *);


static void
leer_directorio(DIR *dir,
                const char *cadena_buscada,
                buscar_cadena buscar_subcadena,
                char *path)
{
	struct dirent *f_actual;
	while ((f_actual = readdir(dir)) != NULL) {
		if (f_actual->d_type == DT_DIR &
		    strcmp(f_actual->d_name, ".") != 0 &
		    strcmp(f_actual->d_name, "..") != 0) {
			// Estoy en un directorio
			// Me copio el path actual
			char path_dir[MAX_PATH];
			strcpy(path_dir, path);
			// concateno el directorio padre con el subdirectorio
			strcat(path_dir, f_actual->d_name);
			if (buscar_subcadena(f_actual->d_name, cadena_buscada) !=
			    NULL) {
				printf("%s\n", path_dir);
			}
			int dir_fd = dirfd(dir);
			int fd_to_be_opened =
			        openat(dir_fd, f_actual->d_name, O_DIRECTORY);
			if (fd_to_be_opened == -1) {
				perror("no se pudo abrir el directorio");
				return;
			}
			// le agrego el "/" al path actual
			strcat(path_dir, "/");

			leer_directorio(fdopendir(fd_to_be_opened),
			                cadena_buscada,
			                buscar_subcadena,
			                path_dir);
			continue;
		} else if (f_actual->d_type == DT_REG) {
			// Estoy en un archivo
			if (buscar_subcadena(f_actual->d_name, cadena_buscada) !=
			    NULL) {
				char path_archivo[MAX_PATH];
				strcpy(path_archivo, path);
				printf("%s\n",
				       strcat(path_archivo, f_actual->d_name));
				continue;
			}
		}
	}
	closedir(dir);
}
static int
validar_parametros(int argc, char *argv[])
{
	// necesariamente el programa debe recibir 2 o 3 argumentos
	if (argc != 2 & argc != 3) {
		return 0;
	}
	// en caso de recibir 3, el segundo debe indicar "-i"
	if (argc == 3 & strcmp(argv[1], "-i") != 0) {
		return 0;
	}
	return 1;
}
int
main(int argc, char *argv[])
{
	if (validar_parametros(argc, argv) == 0) {
		perror("Parametros invalidos");
		return 1;
	}
	const char *cadena_buscada;
	buscar_cadena buscar_subcadena;
	char dir_inicial[MAX_PATH] = "";
	if (argc == 3) {
		cadena_buscada = argv[2];
		buscar_subcadena = strcasestr;
	} else if (argc == 2) {
		cadena_buscada = argv[1];
		buscar_subcadena = strstr;
	}
	DIR *dir = opendir(".");
	if (dir != NULL) {
		leer_directorio(dir, cadena_buscada, buscar_subcadena, dir_inicial);
	} else {
		perror("No se pudo abrir el directorio");
		return 1;
	}
	return 0;
}
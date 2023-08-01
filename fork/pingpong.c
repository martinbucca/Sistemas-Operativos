#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>


int
main()
{
	printf("Hola, soy PID <%d>:\n", getpid());
	//  PIPE IDA (padre a hijo)
	int fds_ida[2];
	int p1 = pipe(fds_ida);
	if (p1 < 0) {
		perror("Error en primer pipe");
		exit(-1);
	}
	printf("  - primer pipe me devuelve: [%d, %d]\n", fds_ida[0], fds_ida[1]);
	// PIPE VUELTA (hijo a padre)
	int fds_vuelta[2];
	int p2 = pipe(fds_vuelta);
	if (p2 < 0) {
		perror("Error en segundo pipe");
		exit(-1);
	}
	printf("  - segundo pipe me devuelve: [%d, %d]\n",
	       fds_vuelta[0],
	       fds_vuelta[1]);

	// REALIZO FORK
	int f = fork();
	if (f < 0) {
		fprintf(stderr, "Error en fork! %d\n", f);
		exit(-1);
	}
	printf("\nDonde fork me devuelve <%d>:\n", f);
	printf("  - getpid me devuelve: <%d>\n", getpid());
	printf("  - getppid me devuelve: <%d>\n", getppid());

	if (f == 0) {
		long int rec_hijo = 0;
		read(fds_ida[0], &rec_hijo, sizeof(rec_hijo));
		printf("  - recibo valor <%ld> vía fd=%d\n", rec_hijo, fds_ida[0]);
		close(fds_ida[0]);
		printf("  - reenvío valor en fd=%d y termino\n", fds_vuelta[1]);
		int segundo_envio =
		        write(fds_vuelta[1], &rec_hijo, sizeof(rec_hijo));
		if (segundo_envio < 0) {
			perror("Error en write! \n");
			exit(-1);
		}
		close(fds_vuelta[1]);
		close(fds_ida[1]);
		close(fds_vuelta[0]);
		exit(-1);
	} else {
		// incicializo el PRNG
		srandom(time(NULL));
		long int val = random();
		printf("  - random me devuelve: <%ld>\n", val);
		printf("  - envío valor <%ld> a través de fd=%d\n",
		       val,
		       fds_ida[1]);
		int primer_envio = write(fds_ida[1], &val, sizeof(val));
		if (primer_envio < 0) {
			perror("Error en write! \n");
			exit(-1);
		}
		close(fds_ida[1]);
		close(fds_ida[0]);
		close(fds_vuelta[1]);
		wait(NULL);

	}
	printf("\nHola, de nuevo PID <%d>:\n", getpid());
	long int rec_padre = 0;
	read(fds_vuelta[0], &rec_padre, sizeof(rec_padre));
	// para que el hijo no quede zombie
	printf("  - recibí valor <%ld> vía fd=%d\n", rec_padre, fds_vuelta[0]);
	close(fds_vuelta[0]);
}
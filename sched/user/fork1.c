#include <inc/lib.h>


void
umain(int argc, char **argv)
{
	envid_t env;
	cprintf("----------------- Welcome to user fork1 test for priority "
	        "scheduler -----------------\n");
	int parent_priority = thisenv->priority;
	int child_priority = 0;  // para incializar
	cprintf("Parent priority: %d\n", parent_priority);
	int f = fork();
	if (f < 0) {
		cprintf("Error in fork\n");
	} else if (f == 0) {
		child_priority =
		        thisenv->priority;  // Obtener la prioridad del proceso hijo
		cprintf("Child priority: %d\n", child_priority);
		if (child_priority > parent_priority) {
			cprintf("OK: Proceso hijo se crea con la misma "
			        "prioridad que el padre y tiene menor "
			        "prioridad que el padre correctamente!\n");
		}
	}
	int new_parent_priority = thisenv->priority;
	if (new_parent_priority > parent_priority) {
		cprintf("OK: Se redujo en uno la prioridad del padre despues "
		        "de ser corrido e interrumpido!\n");
	}
}

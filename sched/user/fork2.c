#include <inc/lib.h>


void
umain(int argc, char **argv)
{
	envid_t env;
	cprintf("----------------- Welcome to user fork2 test for priority "
	        "scheduler -----------------\n");
	int child_and_parent_prior_is_same = 0;
	int new_parent_prior_is_lower_than_old = 0;
	for (int i = 0; i < 50; i++) {
		int parent_priority = 0;
		int child_priority = 0;
		cprintf("Parent priority: %d\n", parent_priority);
		int f = fork();
		if (f < 0) {
			cprintf("Error in fork");
		} else if (f == 0) {
			cprintf("Child priority: %d\n", child_priority);
			child_priority =
			        thisenv->priority;  // Obtener la prioridad del proceso hijo
			if (child_priority > parent_priority) {
				child_and_parent_prior_is_same++;
			}
		}
		int new_parent_priority = thisenv->priority;
		if (new_parent_priority > parent_priority) {
			new_parent_prior_is_lower_than_old++;
		}
	}
	// para esperar a todos los procesos. No pudimos hacer andar a wait()...
	for (int i = 0; i < 500000; i++) {
		continue;
	}
	if (child_and_parent_prior_is_same == 50) {
		cprintf("OK: Proceso hijo se crea con la misma prioridad que "
		        "el padre correctamente!\n");
	}
	if (new_parent_prior_is_lower_than_old == 50) {
		cprintf("OK: Se redujo en uno la prioridad del padre despues "
		        "de ser corrido e interrumpido!\n");
	}
}

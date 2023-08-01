#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


int time_counter_simplifier =
        0;  // cuenta la cantidad de veces que se corrio un proceso. simula el
            // periodo de tiempo S en el cual se reinician los procesos de prioridad
void sched_halt(void);
void search_and_run_next_runnable(int cur_env_pos, int last);
void search_and_run_next_runnable_circular_fashion(int cur_env_pos, int last);
void search_and_run_next_runnable_by_best_priority(int cur_env_pos,
                                                   int last,
                                                   int act_priority);
void search_and_run_next_runnable_by_best_priority_circular_fashion(int cur_env_pos,
                                                                    int last);
void refresh_envs_priorities();
void show_statistics();


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	sched_stats.calls_to_sched++;
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	struct Env *current_env = curenv;
	int actual_position_envs = 0;

	
#ifdef ROUND_ROBIN
	        // starts just after the env this CPU was last running (if is not NULL the curenv)
	        actual_position_envs =
	                curenv ? ENVX(current_env->env_id) + 1 : actual_position_envs;
	        search_and_run_next_runnable_circular_fashion(actual_position_envs, NENV);
	        // no envs are runnable, but the environment previously
	        // running on this CPU is still ENV_RUNNING
	        if (current_env && current_env->env_status == ENV_RUNNING) {
	                env_run(current_env);  // choose this environment
	        }
#endif
	

#ifdef PRIORITY_SCHEDULER
	// explicacion de esta parte: minuto 40:00 clase 19 de mayo
	if (time_counter_simplifier == TIME_SLICE_S_SIMPLIFIER) {
		refresh_envs_priorities();
	}
	search_and_run_next_runnable_by_best_priority_circular_fashion(
	        actual_position_envs, NENV);
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	if (current_env && current_env->env_status == ENV_RUNNING) {
		env_run(current_env);  // choose this environment
	}
#endif


	// Wihtout scheduler, keep runing the last environment while it exists
	//	if (curenv) {
	//		env_run(curenv);
	//	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		show_statistics();  // Las estadísticas deben ser mostradas por
		                    // el kernel al finalizar la ejecución de todos los procesos, durante sched_halt
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

// Searches through envs for a ENV runnable environment from the current env position
// to the last position of the envs array, and runs that env if its status is ENV_RUNNABLE
void
search_and_run_next_runnable(int cur_env_pos, int last)
{
	for (cur_env_pos; cur_env_pos < last; cur_env_pos++) {
		int cur_env_status = envs[cur_env_pos].env_status;
		if (cur_env_status == ENV_RUNNABLE) {
			env_run(&envs[cur_env_pos]);
		}
	}
}

// Searches through envs for a ENV runnable environment in circular fashion
// From the current env (last running in this CPU) to the finish of the array
// and from the start of the array to the current env
void
search_and_run_next_runnable_circular_fashion(int cur_env_pos, int last)
{
	// searches just after the env this CPU was last running
	search_and_run_next_runnable(cur_env_pos, last);
	// returns to the start of the envs array and searches until the env this CPU was last running
	search_and_run_next_runnable(0, cur_env_pos);
}


void
search_and_run_next_runnable_by_best_priority(int cur_env_pos,
                                              int last,
                                              int act_priority)
{
	for (cur_env_pos; cur_env_pos < last; cur_env_pos++) {
		int cur_env_status = envs[cur_env_pos].env_status;
		int cur_env_priority = envs[cur_env_pos].priority;
		if (cur_env_status == ENV_RUNNABLE &&
		    cur_env_priority <= act_priority) {
			time_counter_simplifier++;
			envs[cur_env_pos]
			        .priority++;  // regla 4a:  Si una tarea usa un
			                      // time slice mientras se esta ejecutando
			                      // su prioridad se reduce de una unidad
			sched_stats.total_env_runs++;
			env_run(&envs[cur_env_pos]);
		}
	}
}

/// Pone las prioridades de todos los procesos en 0.
/// Evita que los procesos vayan a starve
void
refresh_envs_priorities()
{
	time_counter_simplifier = 0;
	for (int i = 0; i < NENV; i++) {
		envs[i].priority =
		        0;  // Regla 5: Después de cierto periodo de tiempo S,
		            // se mueven las tareas a la cola con mas prioridad.
	}
	sched_stats.amount_priority_upgrade++;
}


/// Corre el primer proceso empezando por el ultimo que estaba corriendo la CPU
/// y volviendo al pricnipio si no ecuentra y que tenga la mejor prioridad
void
search_and_run_next_runnable_by_best_priority_circular_fashion(int cur_env_pos,
                                                               int last)
{
	int cur_best_priority = 0;
	while (cur_best_priority <= WORST_PRIORITY) {
		search_and_run_next_runnable_by_best_priority(cur_env_pos,
		                                              last,
		                                              cur_best_priority);
		search_and_run_next_runnable_by_best_priority(0,
		                                              cur_env_pos,
		                                              cur_best_priority);
		cur_best_priority++;
	}
}


void
show_statistics()
{
	int env_ids[NENV];
	cprintf("--------------- PRIORITY SCHEDULER STATISTICS --------------- "
	        "\n");
	cprintf("TOTAL ENVS RUN: %d\n", sched_stats.total_env_runs);
	cprintf("AMOUNT OF TIMES ENVS PRIORITIES WHERE UPGRADED TO BEST "
	        "PRIORITY: %d\n",
	        sched_stats.amount_priority_upgrade);
	cprintf("TOTAL CALLS TO SCHED: %d\n", sched_stats.calls_to_sched);
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs != 0) {
			cprintf("ENV WITH ID -- %d -- RUNNED: %d TIMES\n",
			        envs[i].env_id,
			        envs[i].env_runs);
		}
	}
	cprintf("------------------------------------------------------------- "
	        "\n");
}
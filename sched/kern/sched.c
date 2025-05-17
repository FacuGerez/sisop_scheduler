#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/sched.h>

SchedHistory history_scheduler;

void
update_env_info(struct Env *e)
{
	// Update the environment information
	if (e->run_first_time) {
		e->initial_env = history_scheduler.runs_counter;
		e->run_first_time = false;
	}
	e->run_sched_time++;
}

void
show_sched_history()
{
	// for history_scheduler.envs mostrar los IDS (primera posición) y mostrar
	// cuantas veces runneo accediendo a la segunda posicion mostrar los runs
	// totales, que deberían ser igual a sum(for history_scheduler.envs[1])
	cprintf("Scheduler history:\n");
	for (int i = 0; i < history_scheduler.counter; i++) {
		cprintf("Env ID: %d, Runs: %d, Initial Env: %d, Final Env: "
		        "%d\n",
		        history_scheduler.envs[i].env_id,
		        history_scheduler.envs[i].sched_runs,
		        history_scheduler.envs[i].initial_env,
		        history_scheduler.envs[i].final_env);
	}
	cprintf("Total runs: %d\n", history_scheduler.runs_counter);
}

void
sched_init()
{
	// Initialize the scheduler
	history_scheduler.counter = 0;
	history_scheduler.runs_counter = 0;
}

void
sched_add_env(envInfo *e)
{
	// Add the environment to the history
	if (history_scheduler.counter >= SIZE_ENVS) {
		return;
	}
	history_scheduler.envs[history_scheduler.counter].env_id = e->env_id;
	history_scheduler.envs[history_scheduler.counter].sched_runs =
	        e->sched_runs;
	history_scheduler.envs[history_scheduler.counter].initial_env =
	        e->initial_env;
	history_scheduler.envs[history_scheduler.counter].final_env =
	        history_scheduler.runs_counter;
	history_scheduler.counter++;
}

void
sched_update_priority(struct Env *e)
{
	if (e->priority > MIN_PRIORITY) {
		e->priority--;
	}
}

void sched_halt(void);

void
priority_scheduler()
{
	history_scheduler.runs_counter++;

	if (history_scheduler.runs_counter % RUNS_UNTIL_UPGRADE == 0) {
		// If I have runned enough times, I set the priority of the runneable environments
		// to the maximum priority (like MLFQ does).
		for (int i = 0; i < NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE) {
				envs[i].priority = MAX_PRIORITY;
			}
		}
	}

	struct Env *e = NULL;
	int index_max_priority = -1;
	for (int i = 0; i < NENV; i++) {
		// I look:
		// 1 - For the first runnable job.
		// 2 - If I have one, I check if its priority is the biggest.
		if (envs[i].env_status == ENV_RUNNABLE &&
		    (index_max_priority == -1 ||
		     envs[i].priority > envs[index_max_priority].priority)) {
			index_max_priority = i;
			e = &envs[i];
		}
	}

	if (e) {
		// If I have a runnable job, I decrease its priority (if its not
		// minimum) and then I run it.
		sched_update_priority(e);
		update_env_info(e);
		env_run(e);
		return;
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		// If I haven't a runnable job, I try to run the previous job.
		update_env_info(curenv);
		env_run(curenv);
	}

	// If I haven't more jobs, I go to the halt state.
	sched_halt();
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin
	struct Env *e = NULL;

	int start_index = curenv ? ENVX(curenv->env_id) + 1 : 0;
	for (int i = 0; i < NENV; i++) {
		int idx = (start_index + i) % NENV;
		if (envs[idx].env_status == ENV_RUNNABLE) {
			e = &envs[idx];
			break;
		}
	}
	if (e) {
		update_env_info(e);
		env_run(e);
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		update_env_info(curenv);
		env_run(curenv);
	}
	sched_halt();
#endif

#ifdef SCHED_PRIORITIES

	priority_scheduler();

#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		update_env_info(curenv);
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
	panic("No runnable environments!");
	// sched_halt never returns
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
		show_sched_history();
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

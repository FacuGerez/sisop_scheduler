#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	for (int i = 0; i < 5; i++) {
		cprintf("Hello, I am environment %08x, cpu %d, priority %d\n",
		        thisenv->env_id,
		        thisenv->env_cpunum,
		        sys_get_env_priority());
		sys_yield();
		cprintf("Back in environment %08x, priority %d, cpu %d\n",
		        thisenv->env_id,
		        sys_get_env_priority(),
		        thisenv->env_cpunum);
	}
	int priority = sys_get_env_priority();

	cprintf("Get env priority: %d\n", priority);
	int target = priority - 3 >= MIN_PRIORITY ? priority - 3 : MIN_PRIORITY;
	sys_set_env_priority(target);


	cprintf("Set env priority: %d\n", target);

	uint16_t new_priority = sys_get_env_priority();

	cprintf("New priority: %d\n", new_priority);

	if (new_priority != target) {
		panic("Set env priority is not working as expected. Expected: "
		      "%d, Got: %d\n",
		      target,
		      new_priority);
	}
}

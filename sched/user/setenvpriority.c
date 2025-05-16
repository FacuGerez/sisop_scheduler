#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int priority = sys_get_env_priority();
	cprintf("Current priority: %d\n", priority);

	sys_set_env_priority(priority - 3 >= MIN_PRIORITY ? priority - 3
	                                                  : MIN_PRIORITY);

	cprintf("Set env priority: %d\n",
	        priority - 3 >= MIN_PRIORITY ? priority - 3 : MIN_PRIORITY);

	uint16_t new_priority = sys_get_env_priority();

	cprintf("New priority: %d\n", new_priority);

	if (priority > new_priority) {
		panic("Set env priority is not working as expected\n");
	}
}

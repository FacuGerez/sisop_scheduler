#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int priority = sys_get_env_priority();
	cprintf("Current priority: %d\n", priority);

	int target = priority - 3 >= MIN_PRIORITY ? priority - 3 : MIN_PRIORITY;
	sys_set_env_priority(target);

	cprintf("Set env priority: %d\n", target);

	uint16_t new_priority = sys_get_env_priority();

	cprintf("New priority: %d\n", new_priority);

	if (new_priority != target) {
		panic("Set env priority is not working as expected. Expected: %d, Got: %d\n", target, new_priority);
	}
}

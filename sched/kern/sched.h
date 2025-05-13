/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <kern/env.h>
#define SIZE_ENVS 10000

struct sched_history {
    // We save first the env_id and in the second position the times it runs bc we dont want to make O(n²) the show_history algorithm
    env_info envs[SIZE_ENVS];
    int counter;
    int runs_counter;
} typedef sched_history;

extern sched_history history_scheduler;

void sched_init();

void sched_add_env(env_info *e);

void show_sched_history();

// This function does not return.
void sched_yield(void) __attribute__((noreturn));

#endif  // !JOS_KERN_SCHED_H

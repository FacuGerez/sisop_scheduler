/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#define SIZE_ENVS 10000

struct HistoryScheduler {
    // We save first the env_id and in the second position the times it runs bc we dont want to make O(n²) the show_history algorithm
    int envs[SIZE_ENVS][2];
    int runs;
};

// This function does not return.
void sched_yield(void) __attribute__((noreturn));

#endif  // !JOS_KERN_SCHED_H

/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"

#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     10000000         /* 10 seconds */
#define MLFQ_LEVEL_RUNTIME(x) (x + 1) * 100000 /* e.g., 100ms for level 0 */
static ulonglong MLFQ_last_reset_time = 0;
extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Preemptive Scheduler | System Call). */

            /* Initialization of lifecycle statistics, MLFQ or process sleep. */
            proc_set[i].creation_time = mtime_get();
            proc_set[i].response_time_microseconds = 0;
            proc_set[i].cpu_time_microseconds = 0;
            proc_set[i].interrupt_count = 0;
            proc_set[i].start_time = 0;
            proc_set[i].mlfq_level = 0;
            proc_set[i].mlfq_remaining_runtime_microseconds = MLFQ_LEVEL_RUNTIME(0);

            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void print_lifecycle_statistics(int pid) {
    int termination_time = mtime_get();
    struct process *current;

    for (uint i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].pid == pid) {
            current = &proc_set[i];
            break;
        }
    }
    
    printf("Process %d terminated after %d timer interrupts, turnaround time: %dms, response time: %dms, CPU time: %dms\r\n",
        pid,
        current->interrupt_count,
        (termination_time - current->creation_time) / 1000,
        current->response_time_microseconds / 1000,
        current->cpu_time_microseconds / 1000
    );
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
        print_lifecycle_statistics(pid);
    } else {
        /* Free all user processes. */
        for (uint i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
                print_lifecycle_statistics(pid);
            }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */
    p->mlfq_remaining_runtime_microseconds -= runtime;

    if (p->mlfq_remaining_runtime_microseconds <= 0) {
        if (p->mlfq_level < MLFQ_NLEVELS - 1) {
            p->mlfq_level++;
        }
        p->mlfq_remaining_runtime_microseconds = MLFQ_LEVEL_RUNTIME(p->mlfq_level);
    }

    /* Update the MLFQ-related fields in struct process* p after this
     * process has run on the CPU for another runtime microseconds. */


    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
        for (uint i = 0; i < MAX_NPROCESS; i++) {// @TODO: 0 or 1?
            if (proc_set[i].pid <= GPID_SHELL) {
                proc_set[i].mlfq_level = 0;
                proc_set[i].mlfq_remaining_runtime_microseconds = MLFQ_LEVEL_RUNTIME(0);
                break;
            }
        }
    }

    if (MLFQ_last_reset_time < MLFQ_RESET_PERIOD) {
        return;
    }

    MLFQ_last_reset_time = 0;
    /* Reset the level of all processes every MLFQ_RESET_PERIOD microseconds. */
    for (uint i = 0; i < MAX_NPROCESS; i++) {// @TODO: 0 or 1?
        proc_set[i].mlfq_level = 0;
        proc_set[i].mlfq_remaining_runtime_microseconds = MLFQ_LEVEL_RUNTIME(0);
    }

    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the sleep-related fields in the struct process for process pid. */

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */

    /* Student's code ends here. */
}

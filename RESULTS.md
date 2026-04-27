# Assignment 5 Results

## Context Switching Approach

The user-level threading library creates cooperative threads in one xv6 process. Each thread has its own heap-allocated stack and a saved `struct context` containing the callee-saved registers used by `uswtch.S`.

`thread_create()` allocates a stack, places an initial context at the top of that stack, and sets the saved return address to `thread_stub()`. The first time the scheduler switches to that context, `uswtch()` restores the saved registers and returns into `thread_stub()`, which calls the thread function.

`thread_yield()` performs round-robin scheduling over runnable threads. It saves the current thread's stack pointer through `uswtch(&old, new)` and loads the next runnable thread's saved stack pointer. `thread_join()` cooperatively yields until the target thread reaches the zombie state, then frees that thread's stack.

## Mutex Approach

The mutex is cooperative. `mutex_lock()` repeatedly checks the lock and calls `thread_yield()` while the lock is held. Because threads only switch when they explicitly yield, no hardware atomic instruction is needed for this assignment.

## Limitations

- Maximum threads: 8 total, including the main thread.
- Stack size: 4096 bytes per created thread.
- Scheduling is cooperative only; a thread that never yields can prevent other threads from running.
- The mutex is intended for this cooperative threading library, not for preemptive kernel threads.

## Test Result

The `test_pc` producer-consumer program creates two producers and one consumer, uses the mutex to protect an 8-slot bounded buffer, joins all threads, and prints progress every 100 consumed items.

Successful QEMU output:

```text
consumer got 100 items (last=100099)
consumer got 200 items (last=100199)
consumer got 300 items (last=200099)
consumer got 400 items (last=200199)
test_pc: done
```

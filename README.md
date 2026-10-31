*This project has been created as part of the 42 curriculum by slenti.*

# Codexion

## Description

Codexion is a multithreaded C simulation of a resource-allocation problem inspired by the Dining Philosophers challenge, but extended with stricter time constraints, cooldown periods, and dynamic arbitration policies.

In this project, several coders act as concurrent threads. Each coder alternates through the same life cycle:

- acquire two dongles,
- compile,
- release the dongles,
- debug,
- refactor,
- and repeat until the simulation ends.

The global goal is to prevent burnout while ensuring that each coder keeps compiling enough times to satisfy the required total number of completed compilation cycles. A coder burns out if they do not start a new compilation before their deadline (`last_compile_start + time_to_burnout`).

The simulation includes two scheduling policies for the dongles:

- FIFO: requests are served in order of arrival.
- EDF: requests are served according to the earliest deadline first.

The implementation is built with POSIX threads and synchronization primitives, and it includes a monitor thread dedicated to detecting burnout in a timely and precise way.

## Project goals

This project addresses several core concurrency concepts:

- resource sharing under contention,
- deadlock avoidance,
- starvation prevention,
- cooldown enforcement,
- priority-based scheduling,
- precise event logging under concurrency,
- and proper thread lifecycle management.

## Instructions

### Requirements

The project is written in C and must compile with the required flags:

- `-Wall`
- `-Wextra`
- `-Werror`
- `-pthread`

### Compilation

From the project root:

```bash
make
```

Available standard rules:

```bash
make all
make clean
make fclean
make re
```

### Execution

The program expects the following arguments:

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

or, equivalently, the key/value form used by the project usage text:

```bash
./codexion --number_of_coders 4 --time_to_burnout 1500 --time_to_compile 200 --time_to_debug 200 --time_to_refactor 200 --number_of_compiles_required 3 --dongle_cooldown 100 --scheduler fifo
```

Accepted schedulers:

- `fifo`
- `edf`

Simulation behavior:

- The program stops when all coders have completed the required number of compilations.
- The program also stops immediately if a coder burns out.
- Each state change is logged with a timestamp in milliseconds.

### Example

```bash
make
./codexion 4 1500 200 200 200 3 100 fifo
```

## Architecture overview

The implementation follows the project specification with a clear separation between the simulation, the coders, and the monitor:

- `main.c`: argument parsing and entry point.
- `simulation.c`: initialization of shared simulation state and thread startup.
- `coder_routine.c`: coder lifecycle and state transitions.
- `dongles.c`: dongle creation and cleanup.
- `adquire.c`: resource acquisition logic, cooldown checks, and queue management.
- `heap.c`: custom priority queue used to determine which coder is served next.
- `monitor.c`: burnout detection thread.
- `validate.c`: input validation.
- `logger.c`: serialized logging.

The design keeps the simulation deterministic enough for concurrency testing while respecting real-time constraints.

## Blocking cases handled

This implementation explicitly addresses the main concurrency hazards that appear in this problem.

### 1. Deadlock prevention

A deadlock can occur if two or more coders hold one resource and wait forever for another. To avoid this, the project organizes access to the dongles around a single scheduler admission phase:

- each coder registers a request for both dongles,
- the wait queue is evaluated under the selected policy,
- only when a coder is eligible does it continue to acquisition,
- and the acquisition of both dongles is treated as a single coordinated step.

This prevents the classic circular wait pattern where each coder holds one dongle and waits for the other.

### 2. Starvation prevention and fairness

The scheduler prevents a coder from being perpetually bypassed:

- under `fifo`, requests are served in arrival order,
- under `edf`, the earliest-deadline request is prioritized,
- the implementation uses a custom heap structure instead of a standard library priority queue, as required by the subject.

These policies guarantee that request ordering is explicit and deterministic.

### 3. Cooldown handling

After a dongle is released, it cannot be immediately reused. The code stores the cooldown timestamp and checks it before granting access again. This prevents a fast re-acquisition loop that would otherwise cause unfair resource churn and reduce the effective scheduling quality.

### 4. Precise burnout detection

A monitor thread checks elapsed time against each coder’s last compile time. If a coder exceeds its deadline, the monitor emits the burnout log as quickly as possible and stops the simulation.

This is intentionally separated from the coder threads to keep the failure detection independent from the execution of the coders themselves.

### 5. Log serialization

Because multiple threads emit state transitions concurrently, the output must never interleave on a single line. The logger is protected by a mutex, which serializes all writes to stdout and ensures that messages remain readable and correctly ordered.

### 6. Resource leaks and cleanup

All allocated resources are cleaned up at the end of the simulation:

- mutexes are destroyed,
- condition variables are destroyed,
- dongles are freed,
- coder arrays are released,
- and the simulation state is cleared.

## Thread synchronization mechanisms

The implementation relies on several standard POSIX synchronization primitives.

### `pthread_mutex_t`

Mutexes protect shared state that must be updated atomically:

- the global log output,
- the scheduler state and queue,
- the state transitions used by the monitor,
- and each individual dongle’s lock state.

This prevents races such as two coders simultaneously reading or writing the same shared resource state.

### `pthread_cond_t`

Condition variables are used to block a thread until a resource becomes available or a cooldown expires.

Typical flow:

1. A coder registers its request for a dongle.
2. It checks whether the dongle is taken, cooling down, or not the current valid winner.
3. If not immediately eligible, it waits on the condition variable.
4. When a dongle is released or a time threshold changes, a broadcast wakes waiting coders.
5. The awakened coders re-check their conditions and continue only if they are eligible.

This is critical for avoiding busy-waiting and for coordinating the thread wake-up sequence.

### Custom waiting structure and scheduler priority queue

Each dongle owns a small custom heap-like request structure. It stores the coder identifiers and their scheduling priority:

- for FIFO: arrival time,
- for EDF: the deadline (`last_compile_start + time_to_burnout`).

This queue is the core of the arbitration policy. It avoids standard library priority queue usage and keeps the ordering logic explicit and project-compliant.

### Monitor and coder communication

The monitor thread communicates shutdown information to the coders via a shared running flag and scheduler condition broadcast. When a burnout is detected, the monitor stops the simulation and wakes the waiting threads so that they leave the waiting state cleanly and do not remain blocked forever.

### Example of race prevention

A race would exist if two coders both read the same dongle state and both decide it is available at the same time. The project prevents this by protecting access with mutexes and by controlling queue insertion and release through synchronized paths. Only one thread at a time can modify the wait queue or the resource state relevant to the arbitration decision.

## Resources

### References
- "Chapter 4: Threads" in *Mitchell, M., Samuel, A., & Oldham, J. (2001). Advanced Linux programming. New Riders Publishing.*
- [Multithreading in C](https://www.geeksforgeeks.org/c/multithreading-in-c/)
- [Introduction to Min-Heap](https://www.geeksforgeeks.org/dsa/introduction-to-min-heap-data-structure/)
- [POSIX threads - Linux man page](https://www.man7.org/linux/man-pages/man7/pthreads.7.html)
- [Dining Philosophers Problem](https://www.geeksforgeeks.org/operating-systems/dining-philosophers-problem/)

### AI usage

AI was used as an auxiliary tool during this project for:

- reasoning about the concurrency model and edge cases,
- validating the correctness of the shutdown and burnout logic,
- clarifying synchronization patterns and fairness concerns,
- reviewing the README structure to ensure the subject requirements were fully covered,
- and helping structure the explanation of FIFO/EDF arbitration and burnout detection.

The final implementation and correctness decisions were still verified against the project requirements and the behavior of the code itself.

## Notes on implementation choices

The project uses a monitor-based design, which is useful for precise event detection in concurrent simulations. The separate monitor thread ensures that the burnout check is not entangled with the normal coder state machine, which makes the logic clearer and easier to reason about.

The scheduler is intentionally simple but effective: each dongle is considered as a shared resource with a small waiting queue, and the priority structure determines which waiting coder may continue. This is a direct fit for the subject’s emphasis on fairness, real-time deadlines, and bounded waiting.

## Summary

Codexion is a practical concurrency challenge that combines classic synchronization problems with real-time deadlines and scheduling policies. It demonstrates how mutexes, condition variables, custom queues, and a dedicated monitor thread can work together to manage a shared resource system while preventing common failure modes such as deadlock, starvation, and inconsistent logging.

---

This project is intended as both a concurrency exercise and a demonstration of safe multithreaded design in C.

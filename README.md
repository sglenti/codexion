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

The global goal is to prevent burnout while ensuring that each coder keeps compiling enough times to satisfy the required total number of completed compilation cycles. A coder burns out if they do not start a new compilation before their deadline (`last_compile_time + time_to_burnout`).

The simulation includes two scheduling policies for the dongles:

- FIFO: requests are served in order of arrival.
- EDF: requests are served according to the earliest deadline first.

The implementation is built with POSIX threads and synchronization primitives, and it includes a monitor thread dedicated to detecting burnout in a timely and precise way.
 
This project addresses several core concurrency concepts: resource sharing under contention, deadlock avoidance, starvation prevention, cooldown enforcement, priority-based scheduling, precise event logging under concurrency,
and proper thread lifecycle management.

## Instructions

### Requirements

The project is written in C and must compile with the required flags:

`-Wall` `-Wextra` `-Werror` `-pthread`

### Compilation

From the project root:

```bash
make
```

Available standard rules:

```bash
make          # build
make clean    # remove object files
make fclean   # remove object files and executable
make re       # clean and rebuild
```

An optional flag allows to set at compile time the maximum number of coders accepted by the simulation (the default is 256):

```bash
make MAX_CODERS=128
### Execution
```

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
- `adquire1.c/adquire2.c`: resource acquisition logic, cooldown checks, and queue management.
- `heap.c`: custom priority queue used to determine which coder is served next.
- `monitor.c`: burnout detection thread.
- `validate.c`: input validation.
- `logger.c`: serialized logging.

## Blocking cases handled

### 1. Deadlock prevention

## Blocking and concurrency hazards

Codexion coordinates access to dongles with per-dongle mutexes and condition
variables. The scheduler also records each coder's request in the wait queues.
The design explicitly addresses the main synchronization risks in this problem.



### Deadlock prevention

Coffman's conditions describe four conditions that must all hold for a
deadlock to occur:

1. **Mutual exclusion** — a resource can be held by only one thread at a time.
2. **Hold and wait** — a thread holds one resource while waiting for another.
3. **No preemption** — a resource cannot be forcibly taken from its holder.
4. **Circular wait** — threads form a cycle, each waiting for a resource held
   by the next thread.

A deadlock can occur when one coder holds one dongle and waits for another,
while another coder does the opposite. To avoid this, each coder treats the
acquisition of its two dongles as a single coordinated operation: it checks the
state of both dongles, evaluates the scheduler priority, and then claims both
under their mutexes in a fixed order.

The critical rule is that all code paths acquiring multiple dongles must do so
in the same lock order. This prevents circular waits between competing coders.

To keep that lock order consistent around the coder/dongle ring, coder 1's
dongle pointers are switched during initialization. As a result, every coder
locks its lower-numbered dongle first and its higher-numbered dongle second.
This prevents competing coders from acquiring the same pair of mutexes in
opposite orders, which could otherwise create a circular wait.

### Waiting and cooldown

A coder that cannot proceed waits on its dongle condition variable instead of
busy-waiting. Releasing a dongle broadcasts to waiters, but the awakened coder
must re-check the state before proceeding: the resource may still be unavailable
or the cooldown may not have expired.

### Request ordering

Each dongle owns a compact request queue. FIFO serves requests by arrival time,
while EDF prioritizes the request with the earliest effective deadline. The
scheduler selects which waiting coder is allowed to proceed, and the selected
coder is the one whose request is at the front of the relevant heap.

### TOCTOU race prevention

A TOCTOU (time-of-check to time-of-use) race occurs when a thread evaluates the
state of the shared resources and then acts on that result after another thread
has changed the same state. In the current implementation, the check and the
claim are protected by the same dongle mutexes, and the code does not release
either mutex in between.

This means the decision to proceed is both atomic and consistent with the
resource state: the request is validated, then the reservation is made while
still holding the mutexes. The race is therefore avoided by making the
check-and-claim a single critical section instead of two separate actions.

### Shutdown and burnout

A monitor thread checks each coder's elapsed time since its last compilation.
When a burnout is detected, the simulation is stopped and waiting coders are
notified so they can re-check the running state and exit safely. Condition
variables are used with a predicate-based wait protocol, so wake-ups do not
cause false progress when the required condition is still unsatisfied.

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
- for EDF: the deadline (`last_compile_time + time_to_burnout`).

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

# CPRE 458

Real time system: system with both performance and time requirements, operation must be predictable. Typically used in safety-critical systems or controls, but can be used in everything down to telecom (people don't like lag in their calls)

Penalty/reward system: there are rewards for making the deadline and penalties for missing it. Hard RTOS has a very high penalty, firm RTOS has a medium penalty, soft RTOS has a gradually increasing small penalty

Task types:
- Periodic
  - Characterized by computation time and period $T = (c, p)$
  - Known beforehand
- Aperiodic (event-driven, like an interrupt)
  - Characterized by arrival time, ready time, computation time, deadline $T = (a, r, c, d)$
  - Not known beforehand
- Sporadic
  - Known minimum inter-arrival time of a periodic task. Basically computing in bursts. We know the worst-case interval

Constraints:
- Deadline
- Resources
- Precedence
- Fault tolerance

## Predictability
Predictability: With certain assumptions about workload and failure rate, it should be possible to show at design time that the timing constraints of the application will be met.
- For static systems, predictability can always be determined at design time
- For dynamic systems, predictability cannot be guaranteed because task characteristics aren't known beforehand
  - However, once a dynamic task is admitted to the system the predictability and timing guarantees should always hold. This is based on some real-time admission test to make sure we have enough extra performance available

## Scheduling
Admission control has two parts: schedulability check (can we run it), schedule construction (when do we run it)

- Static system
  - Offline/Offline: Table-driven
  - Offline/Online: Priority-driven
- Dynamic
  - Online/Online: Planning-based
  - Best effort

Preemptive scheduling: task execution is paused while higher priority tasks are executed and resumed later. Lots of context switching, but lots of "schedulability" (ability to have tasks execute exactly on time), but lots of context switching loss. Example: round robin

Non preemptive scheduling: once a task starts, it must finish. Lower schedulability, but lower losses due to context switching

Harmonic task set: All tasks in the set have periods that are multiples of one another. Ex. 2, 4, 8.

### Aperiodic tasks
We care about the schedulability of periodic tasks and the response time of aperiodic tasks.

Aperiodic tasks are executed when there are no periodic tasks to execute. The obvious way to schedule aperiodic tasks is using fixed "holes" in the schedule, but this can result in long response time and missed deadlines.

Solution: create a "server" to handle aperiodic tasks. These are real tasks in the schedule that are scheduled based on the real aperiodic workload that handle aperiodic tasks during their execution time.

- Polling server: Handles aperiodic tasks during its execution time, stalls if there's nothing to be handled. Poor response time for aperiodic tasks.
- Deferrable server: Has a "bank" of compute time to use anytime during its period. It can preempt any task to execute its aperiodic task (until it's out of compute time) and then go back to sleep. The "bank" refills every period.
- Priority exchange server: If there's nothing to execute it swaps priority with a lower priority task for the duration of its slot that also needs to execute at that time.

### RMS
Rate Monotonic Scheduling: Shorter period tasks have higher priorities, and the highest priority job at any point is executed. Allows preemption.

Priorities are static: They are set by the task execution time.

Schedulability check with n *periodic* tasks defined by (Ci, Pi):
- If harmonic: Check if sum of utilizations $\sum c_i / p_i \leq 1$
- $\sum_{i = 1}^{n} c_i / p_i \leq n (2^{1/n} - 1)$
  - Pass: Task is schedulable
  - Fail: Task may be schedulable, needs another test

### EDF/LLF
Earliest Deadline First: Earlier deadlines get higher priority.

Least Laxity First: Lower laxity (deadline - remaining execution time) gets higher priority.

Priorities change as the scheduler runs.

Schedulability check: $\sum c_i / p_i \leq 1$

## Resource Management
Usually done through mutexes. There is a priority queue in the scheduler waiting for resources

**Mutex**: Binary value, locked or unlocked. Only one thread of one process can access a shared resource at once. However, if that thread goes to sleep or has to block on IO while holding the mutex nothing else can access the shared resource.

**Semaphore**: A generalization of a mutex that uses an integer for the number of threads that can access a resource simultaneously. Integer = 1 is just a standard mutex, integer > 1 is special. Uses signal() and wait(), or up() and down() rather than lock() and unlock().
- I have never seen an integer > 1 semaphore used anywhere
- A mutex is a semaphore, but a semaphore isn't necessarily a mutex

Critical section: section that requires a shared resource and must be run quickly.

**Priority inversion**: High priority process is blocked because low priority process is holding onto resources and keeps getting interrupted. High priority process tries to run and fails. This is bad scheduling and system design, we want to prevent this.

**Priority inheritance**: A low priority task has a resource and a high priority task wants to access it. The low priority task inherits the high priority temporarily, executes its task, then goes back to its old priority.
- May lead to deadlock

**Priority inheritance protocol**: Basically a fixed way to handle granting privileges.

**Priority ceiling protocol**: Each mutex/semaphore has a priority ceiling: the highest priority task that can lock it. When a task reaches a critical section, the task will be suspended unless its priority is higher than the highest priority ceiling of all other tasks waiting for that critical section.

Deadlock: One process is waiting for a resource from another, while that process is waiting for a resource from the first. Resources cannot be forcably taken away. A cycle in a dependency/priority graph.
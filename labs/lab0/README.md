Ryan Vo

# Part 0: Repeating Forks

## 4.

0Yo!1Yo!Yo!2Yo!Yo!Yo!3Yo!Yo!Yo!Yo!4Yo!Yo!Yo!Yo!Yo!5Yo!Yo!Yo!Yo!Yo!5%

The printf has a buffer and is not being flushed as we expect.
The Yo's are repeated cause the parent's buffer is not cleared after each fork.

## 7.

[Detaching after fork from child process 40952]
0[Detaching after fork from child process 40953]
Yo!1[Detaching after fork from child process 40954]
Yo!Yo!2[Detaching after fork from child process 40955]
Yo!Yo!Yo!3[Detaching after fork from child process 40956]
Yo!Yo!Yo!Yo!4[Detaching after fork from child process 40957]
Yo!Yo!Yo!Yo!Yo!5Yo!Yo!Yo!Yo!Yo!5[Inferior 1 (process 40948) exited normally]

## 8.

Output buffering is occurring. The "Yo!" strings are getting
buffered and not immediately printed, so when the buffer finally flushes,
multiple Yo's appear at once.

# Part 1: Who Runs First?

## 1. 1000 child parent, 0 parent child

## 2. 90 child parent, 910 parent child

## 3. 100 child parent, 900 parent child

In the first step, without fflush, the child always printed first
because the parent's output was buffered and not flushed before the child executed.
In the second step, with fflush in both parent and child, the parent printed first
most of the time, because the fflush forced the parent's output to be written immediately.
In the third step, the parent still printed first most of the time, write buffer size
of the message and outputs it immediately.

Adding sleep for 0.1 second changed the write()
variant of my program to print parent first 990 times and
child first 10 times. Sleep for 1 second outputted child first 100 times
and parent first 900 times.

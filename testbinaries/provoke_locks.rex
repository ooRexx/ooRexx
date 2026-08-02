/* Driven by the provoke_locks test binary.

   Spawns reply threads so that InterpreterInstance::poolActivity runs on a
   worker, holding the kernel lock and requesting the resource lock, while the
   thread that created the instance is still active. That is one half of the
   lock pair in the stack traces on bug #1734; concurrent instance creation on
   the other threads is the other half.

   The body is the reproducer from that bug report, minus the output. */
.c~s
.c~s
.c~s
.c~s

::class c
::method s class
reply

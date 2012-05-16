
## How to run GDB

GDB is a general-purpose debugger

    gdb --args ./svfs -f example/rootdir/ example/mountdir

## How to run valgrind

valgrind is a tool to debug memory problems (corruption, leaks, etc.)

    valgrind ./svfs -f example/rootdir/ example/mountdir

## Testing

A few interesting commands:

    cd example
    touch mountdir/{a,b,c,a,a,b,a,c,a,b,d,a,a,b,d,d,d,c,a}

(Note: for some reason, vim writes twice. Which means we'll have twice
as many backup files :/ Except if we only write on, say, different sha1)

## When it crashes

    sudo umount -l mountdir

When gdb breaks, you're stuck, don't try to ls, to cd, nothing, just
step through gdb and quit/restart because FUSE will freeze the fuck
out of your system.


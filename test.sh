#!/bin/bash

# Compiles the code, unmounts the old file system and then mounts
# the new version of it.

make

fusermount -u example/mountdir 

./svfs -s example/rootdir example/mountdir


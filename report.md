
# SVFS

## Amos Wenger, Ismaïl Amrani, Sébastien Zurfluh

We were asked to implement a simple user-space filesystem
that does backups on a regular basis, on top of FUSE.

### General observations

FUSE is very simple and very nice to use. You just get hooks
for all the things you need (file open/close/write, same for
directories, etc.): one can understand why there exists so
many FUSE-based filesystems out there, for example for all
the different cloud storage services. It's just a pleasure
to use.

### Implementation notes

#### Intro

The base code we were provided was very clean and it was a
nice foundation to build upon. We quickly decided on a few
things: first, that we would need a hash table to map file paths
to their backups. And second, that, contrary to a limitation
mentioned in the problem specification, backups would be stored
with a singly linked list, to allow any number of backup files,
but also a cleaner data structure.

#### The hash table

The hash function implemented here is the X31 hash: it is a
very fast hash that should not be used for cryptographic
purposes, but which tends to do a very good job at being used
in the context of a lookup table.

We also planned to implement [Cuckoo hashing][cuckoo] to have
a proper hash table but finally, doing a binary research in a
sorted array of keys seemed to be more than enough for such a
project. We maintain the key arrays sorted by inserting the
elements directly in the right place. Note that our structure
is not thread-safe.

[cuckoo]: https://en.wikipedia.org/wiki/Cuckoo_hashing

#### Singly linked lists

Each entry in the hash table corresponds to a set of backups,
stored from newest to oldest: the reason is simple, they are
added as we go, and prepending an element to a singly-linked
list is O(1), whereas appending one is O(n).

#### Debugging

Debugging our file system was very nice once we discovered the
-f and -s options of fuse. There was some confusion at first
as to the order in which options were accepted by the program,
but it went over very well.

It was interesting to notice that a lot more I/O operations
are done than we would intuitively think: for example, most
editors write dummy files before actually saving the real file.
vim writes files in [0-9]{4} format, gedit uses .goutputstream-[A-Z]{5}
files (although I suspect this comes from a higher-level
GLib abstraction).

### Garbage collection

#### Duality

Here's a rundown of what happens in our filesystem: as implemented
in the skeleton we were given, i/o operations in mountdir actually
happen on backing files in rootdir

#### The open hook

When a file is opened with write flags (see `svfs_has_write_flags`),
we look it up in the context hash table. If it doesn't already have
an entry, one is created. Then, a backup is created by copying the
backing file to another file in rootdir. A reference for this backup
is added to the singly linked list of backups contained in the table
entry, and then we return from the FUSE hook.

Note that there is no need to listen for the 'write hook' because
text editors typically close and re-open the file whenever writing
to it. And moreover, these same editors usually buffer their I/O
operations, which would result in way too many backups. By listening
to 'open for writing' only, we guarantee that there is at most one
backup for each save from the user.

A more space-efficient, but costlier (in CPU) approach, would be to
store a hash of the file with each backup in the linked list: then,
when a file is opened, we would check with the last backup to see
if there was any difference, and only create a new backup if it is
the case. Better still, we could go full de-duplication, a-la git,
storing objects referred by their SHA1 (or similar) hash, ensuring
unique content. But this goes well beyond the scope of this exercise.

#### Unobtrusive garbage collection

Upon investigation in the Moodle forums, a proposed method was to
run a background thread in order to trigger the collection every few
seconds. However, this has one major drawback: it would require all
our data structures to be theadsafe. That is a significant hurdle and
would take a great deal of time and care to do that correctly.

Whereas the approach we have taken have none of the disagreements,
and is just as effective: whenever we are in FUSE's open hook, we
run a garbage collection if more than N seconds have elapsed since
the last collection.

In practice, this works very nicely: the excess garbage that this
method generates is practically never a problem, because every time
a file is open for writing, a collection is potentially run, which
means that the space is freed even before we attempt to use it, i.e.
no problem whatsoever.

At the beginning, we thought of implementing garbage collection
with a background thread as a compile-time opt-in solution, but
the more we thought of it, the less convinced we were that it was
really necessary: the unobtrusive approach we have implemented
seems superior in every regard, and most importantly, it is rock-solid:
and for such a crucial part of an OS as a filesystem, we prefer
reliability.

#### Clean-up on exit

When the filesystem is unmounted, we walk through the whole hash
table, and all backups are deleted one by one. Memory is freed on exit.


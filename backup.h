
#ifndef _BACKUP_H_
#define _BACKUP_H_

#include <stdint.h>

#define X31_SEED 0xbafffffe

/* X31 hash implementation */
uint32_t x31 (const uint8_t *key, uint32_t len, uint32_t seed);
uint32_t hash_string(const char *string);

/*
 * Info we store for a backed up file.
 *
 * Created on-demand when a file is written to.
 */

struct v_list {
    char *path;
    struct v_list *next;
};

struct v_backup {
    uint32_t hash;
    char *path;
    int num_writes;
    struct v_list *backups;
};

struct v_backup *v_backup_new(char *path);

/*
 * A weird, sort of simplistic hash table.
 * It's basically just a list of entries, sorted by
 * their keys, to allow O(ln(x)) lookup.
 *
 * Definitely not optimal, but will do the job here
 */

struct v_entry {
    uint32_t key;
    struct v_backup *value;
};

struct v_table {
    uint32_t capacity;
    uint32_t size;
    struct v_entry **entries;
};

/* Create a new hash table */
struct v_table *v_table_new();

/* Print the content of the table */
void v_table_print(struct v_table *table);

/* Insert a new entry in the hash table */
void v_table_insert(struct v_table *table, uint32_t key, struct v_backup *value);

/* Look up an entry in the table */
struct v_backup *v_table_lookup(struct v_table *table, const char *path);

/* Append a new backup to take care of by the GC */
void v_backup_append(struct v_backup *backup, char *path);

/* Count number of backups stored */
int v_backup_count(struct v_backup *backup);

#endif // #ifndef _BACKUP_H_


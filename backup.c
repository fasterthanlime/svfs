
#include "backup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* X31 hash implementation */
uint32_t x31 (const uint8_t *key, uint32_t len, uint32_t seed) {
    uint32_t hash = seed + len;
    for (; len & ~1; len -= 2, key += 2) {
        hash = (((hash * 31) + key[0]) * 31) + key[1];
    }
    if (len & 1) {
        hash = (hash * 31) + key[0];
    }
    return hash;
}

struct v_table *v_table_new() {
    struct v_table *table = calloc(1, sizeof(struct v_table));

    table->capacity = 5;
    table->size = 0;
    table->entries = calloc(table->capacity, sizeof(struct v_entry));

    return table;
}

void v_table_print(struct v_table *table) {
    printf("[vtable] ------------------------ \n");

    for (int i = 0; i < table->size; i++) {
        struct v_entry *entry = table->entries[i];
        int count = v_backup_count(entry->value);
        printf("[vtable] - %x => %s x %d (written %d times)\n", entry->key, entry->value->path, count, entry->value->num_writes);
    }
}


int v_table_binary_search(struct v_table *table, uint32_t key) {
    int index = 0;
   
    if (table->size > 0) {
        int lbound = 0;
        int rbound = table->size;

        //printf("[vtable] should find index for %d\n", key);

        // binary search to find the right spot
        int done = 0;
        do {
            index = lbound + (rbound - lbound) / 2;
            if (index == table->size || lbound == rbound) {
                break; // we're done here
            }

            uint32_t middle = table->entries[index]->key;
            //printf("[vtable] [%u, %u], index = %u, middle = %u\n", lbound, rbound, index, middle);

            if (middle > key) {
                // middle is bigger? look in the left portion
                rbound = index;
                //printf("[vtable] left portion, new range = [%u, %u]\n", lbound, rbound);
                continue;
            } else if(middle < key) {
                // middle is smaller? look in the right portion
                lbound = index + 1;
                //printf("[vtable] right portion, new range = [%u, %u]\n", lbound, rbound);
                continue;
            }
            done = 1;
        } while (!done);
    }

    //printf("[vtable] index for key %d is %d\n", key, index);
    return index;
}

void v_table_grow(struct v_table *table) {
    // totally random heuristic, yay false intuition!
    table->capacity = (table->capacity + 10) * 1.5;
    table->entries = realloc(table->entries, table->capacity * sizeof(struct v_entry *));
}

void v_table_insert(struct v_table *table, uint32_t key, struct v_backup *value) {
    if (table->size + 1 > table->capacity) {
        v_table_grow(table);
    }

    int index = v_table_binary_search(table, key);

    memmove(table->entries + index + 1, table->entries + index, (table->size - index) * sizeof(struct v_entry*));

    table->entries[index] = calloc(1, sizeof(struct v_entry));
    (*table->entries[index]) = (struct v_entry) {
        .key = key,
        .value = value
    };

    table->size++;
}

struct v_backup *v_table_lookup(struct v_table *table, const char *path) {
    int key = hash_string(path);
    int index = v_table_binary_search(table, key);

    if (index >= 0 && index < table->size && table->entries[index]->key == key) {
        struct v_backup *result = table->entries[index]->value;
        if(strcmp(result->path, path) == 0) {
            //printf("[vtable] found %s in table at index %d\n", path, index);
            return result;
        }
    }

    //printf("[vtable] %s is not in the table yet\n", path);
    return NULL;
}

uint32_t hash_string(const char *string) {
    return x31((const uint8_t *) string, strlen(string), X31_SEED);
}

struct v_backup *v_backup_new(char *path) {
    struct v_backup *backup = calloc(1, sizeof(struct v_backup));
    (*backup) = (struct v_backup) {
        .hash = hash_string(path),
        .path = path,
        .num_writes = 0,
        .backups = NULL,
    };

    return backup;
}

int v_backup_count(struct v_backup *backup) {
    int count = 0;
    struct v_list *list = backup->backups;

    while (list) {
        list = list->next;
        count++;
    }

    return count;
}

void v_backup_append(struct v_backup *backup, char *path) {
    struct v_list **target = NULL;

    if (!backup->backups) {
        target = &(backup->backups);
    } else {
        struct v_list *tail = backup->backups;
        while (tail->next) {
            tail = tail->next;
        }
        target = &(tail->next);
    }

    *target = calloc(1, sizeof(struct v_list));
    (**target) = (struct v_list) {
        .path = path,
        .next = NULL,
    };

    return;
}



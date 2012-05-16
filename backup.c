
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
    printf("[vtable] Table with %d elements and capacity %d:\n", table->size, table->capacity);

    for (int i = 0; i < table->size; i++) {
        printf("[vtable] - Entry %d = %u => %s\n", i, table->entries[i]->key, table->entries[i]->value->path);
    }
    printf("[vtable] ------------------------ \n");
}


int v_table_binary_search(struct v_table *table, uint32_t key) {
    int index = 0;
   
    if (table->size > 0) {
        int lbound = 0;
        int rbound = table->size;

        printf("[vtable] should find index for %d\n", key);

        // binary search to find the right spot
        int done = 0;
        do {
            index = lbound + (rbound - lbound) / 2;
            if (index == table->size || lbound == rbound) {
                break; // we're done here
            }

            uint32_t middle = table->entries[index]->key;
            printf("[vtable] [%u, %u], index = %u, middle = %u\n", lbound, rbound, index, middle);

            if (middle > key) {
                // middle is bigger? look in the left portion
                rbound = index;
                printf("[vtable] left portion, new range = [%u, %u]\n", lbound, rbound);
                continue;
            } else if(middle < key) {
                // middle is smaller? look in the right portion
                lbound = index + 1;
                printf("[vtable] right portion, new range = [%u, %u]\n", lbound, rbound);
                continue;
            }
            done = 1;
        } while (!done);
    }

    printf("[vtable] index for key %d is %d\n", key, index);
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
    v_table_print(table);
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
    };

    return backup;
}


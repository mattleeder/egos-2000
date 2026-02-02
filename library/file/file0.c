/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: a simple (if not naive) file system
 */

#ifdef MKFS
#include <stdio.h>
#include <sys/types.h>
#else
#include "egos.h"
#endif

#include "inode.h"
#include <stdlib.h>
#define DUMMY_DISK_OFFSET(ino, offset) ino * 128 + offset

/* Student's code goes here (File System). */
#include "disk.h"
#include <string.h>

#define INODES_PER_BLOCK        (BLOCK_SIZE / sizeof(struct inode))
#define FAT_ENTRIES_PER_BLOCK   (BLOCK_SIZE / sizeof(struct entry))

/* Include necessary header files; Define data structures for your file system;
   Define helper functions; You can also put these to a separate header file. */

struct inode {
    int size, head;
};

struct inode_block {
    struct inode inodes[INODES_PER_BLOCK];
};

struct entry {
    int next;
};

struct fat_table_block {
    struct entry entries[FAT_ENTRIES_PER_BLOCK];
};

struct data_block {
    char data[BLOCK_SIZE];
};

#define SUPERBLOCK_SIZE 512

static struct superblock {
    int iblock_count;
    int fblock_count;
    int head;
    unsigned char padding[SUPERBLOCK_SIZE - 3 * sizeof(int)];
} SB;

_Static_assert(sizeof(struct superblock) == 512,
               "superblock must be exactly 512 bytes");


/* Student's code ends here. */

int get_inode_block_number(uint ino) {
    return (ino / INODES_PER_BLOCK) + 1; // + 1 for super block
}

struct inode *get_inode_from_inode_block(uint ino, struct inode_block *block) {
    return &block->inodes[ino % INODES_PER_BLOCK];
}

int get_fat_entry_block_number(uint fat_entry) {
    return SB.iblock_count + 1 + (fat_entry / FAT_ENTRIES_PER_BLOCK);
}

struct entry *get_fat_entry_from_fat_table_block(uint fat_entry_number, struct fat_table_block *block) {
    return &block->entries[fat_entry_number % FAT_ENTRIES_PER_BLOCK];
}

int get_data_block_number(uint data_block_number) {
    return data_block_number + SB.iblock_count + SB.fblock_count + 1;
}

int fat_walk(inode_intf below, int steps, struct entry **fat_entry, struct fat_table_block *fat_block) {
    int fat_block_number;
    for (int i = 0; i < steps; i++) {
        
        if (*fat_entry == NULL || (*fat_entry)->next < 0) {
            return -1;
        }

        fat_block_number = get_fat_entry_block_number((*fat_entry)->next);
        if(below->read(below, 0, fat_block_number, (block_t *)fat_block) < 0) {
            return -1;
        }

        *fat_entry = get_fat_entry_from_fat_table_block((*fat_entry)->next, fat_block);
    }

    return 0;
}

int mydisk_read(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (File System). */

    /* Replace the code below with your own file system read logic. */
    inode_intf below = self->state;

    // Read superblock
    if (below->read(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    struct inode_block iblock;
    struct inode *inode_to_read;

    int inode_block_number = get_inode_block_number(ino);
    if (below->read(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    } 

    inode_to_read = get_inode_from_inode_block(ino, &iblock);
    if (offset >= inode_to_read->size) {
        return -1;
    }

    struct fat_table_block  fat_block;
    struct entry            dummy_entry = {.next = inode_to_read->head};
    struct entry            *fat_entry = &dummy_entry;
    uint                    fat_block_number;

    if (fat_walk(below, offset, &fat_entry, &fat_block) < 0) {
        return -1;
    }

    return below->read(below, 0, get_data_block_number(fat_entry->next), block);
    /* Student's code ends here. */
}

int mydisk_write(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (File System). */

    inode_intf below = self->state;
    
    // Read superblock
    if (below->read(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    // Read inode
    struct inode_block iblock;
    struct inode *inode_to_write;

    int inode_block_number = get_inode_block_number(ino);
    if (below->read(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    }
    
    inode_to_write = get_inode_from_inode_block(ino, &iblock);
    
    // Move along linked list to offset or tail
    uint num_blocks_to_allocate = offset - inode_to_write->size + 1;
    
    uint inode_head = inode_to_write->head == -1 ? SB.head : inode_to_write->head;
    int num_jumps = offset >= inode_to_write->size ? inode_to_write->size : offset;

    struct fat_table_block  fat_block;
    struct entry            dummy_fat_entry = {.next = inode_head};
    struct entry            *fat_entry = &dummy_fat_entry;
    uint                    fat_block_number;

    if (fat_walk(below, num_jumps, &fat_entry, &fat_block) < 0) {
        return -1;
    }

    uint data_block_to_write = fat_entry->next;
    if (num_blocks_to_allocate < 0) {
        return below->write(below, 0, get_data_block_number(data_block_to_write), block);
    }
        
    uint old_head = SB.head;

    // 1. If inode->head == -1, set equal to SB.head, else Point inode_tail to head of free list
    // 2. Traverse free list for (offset - size + 1) nodes
    // 3. Point SB.head = free[offset - size + 1].next
    // 4. Point free[offset - size + 1] = -1
    // 5. Update inode size and write to disk

    // 1.
    if (inode_to_write->head == -1) {
        fat_entry->next = SB.head;
        inode_to_write->head = SB.head;
    } else {

        // Read in fat block if we didnt already
        if (num_jumps <= 0) {
            if (below->read(below, 0, get_fat_entry_block_number(fat_entry->next), (block_t *)&fat_block) < 0) {
                return -1;
            }
            fat_entry = get_fat_entry_from_fat_table_block(fat_entry->next, &fat_block);
        }

        // Point inode_tail to head of free list and write updated fat block to disk
        fat_entry->next = SB.head;
        if (below->write(below, 0, fat_block_number, (block_t *)&fat_block) < 0) {
            return -1;
        }
    }

    // 2. Traverse free list for (offset - size + 1) nodes
    fat_walk(below, num_blocks_to_allocate - 1, &fat_entry, &fat_block);
    data_block_to_write = fat_entry->next;
    fat_walk(below, 1, &fat_entry, &fat_block);
    fat_block_number = get_fat_entry_block_number(data_block_to_write);

    // 3. Point SB.head = free[offset - size + 1].next and update superblock
    SB.head = fat_entry->next;
    if (below->write(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    // 4. Point free[offset - size + 1] = -1
    fat_entry->next = -1;
    if (below->write(below, 0, fat_block_number, (block_t *)&fat_block) < 0) {
        return -1;
    }
    
    // 5. Update inode size and write to disk
    inode_to_write->size +=  offset - inode_to_write->size + 1;
    if (below->write(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    }

    
    return below->write(below, 0, get_data_block_number(data_block_to_write), block);
    /* Student's code ends here. */
}

int mydisk_getsize(inode_intf self, uint ino) {
    /* Student's code goes here (File System). */
    inode_intf below = self->state;

    // Read superblock
    if (below->read(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    struct inode_block iblock;
    struct inode *inode_to_read;

    int inode_block_number = get_inode_block_number(ino);
    if (below->read(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    }

    inode_to_read = get_inode_from_inode_block(ino, &iblock);
    return inode_to_read->size;
    
    /* Replace the code below with code for getting the size of an inode. */
    /* Student's code ends here. */
}

int mydisk_setsize(inode_intf self, uint ino, uint nblocks) {
    /* Student's code goes here (File System). */
    inode_intf below = self->state;
    
    // Read superblock
    if (below->read(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    // Read inode
    struct inode_block iblock;
    struct inode *inode_to_write;

    int inode_block_number = get_inode_block_number(ino);
    if (below->read(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    }

    inode_to_write = get_inode_from_inode_block(ino, &iblock);


    // Already have the right amount
    if (inode_to_write->size == nblocks) {
        return 0;
    }

    // Give nodes back
    if (inode_to_write->size > nblocks) {
        // Walk nblocks steps along fat, fat_entry is the last node we will keep
        // Store old_head = SB.head
        // Set SB.head = fat_entry->next
        // If nblocks == 0 set inode_to_write->next = -1
        // Walk inode_to_write->size - nblocks, this is the tail node of inode_to_write
        // Set fat_entry->next = old_head
        // Write inode_block
        // Write superblock
        // Write fatblock

        uint number_of_blocks_to_return = inode_to_write->size - nblocks;

        struct fat_table_block  fat_block;
        struct entry            dummy_fat_entry = {.next = inode_to_write->head};
        struct entry            *fat_entry = &dummy_fat_entry;
        uint                    fat_block_number;

        // Walk to last node
        fat_walk(below, nblocks, &fat_entry, &fat_block);

        uint sb_old_head = SB.head;
        SB.head = fat_entry->next;

        // Keep zero nodes
        if (nblocks == 0) {
            inode_to_write->head = -1;
        }

        // Walk to tail node
        fat_walk(below, number_of_blocks_to_return, &fat_entry, &fat_block);

        fat_entry->next = sb_old_head;

        // Write inode_block
        if (below->write(below, 0, get_inode_block_number(ino), (block_t *)&iblock) < 0) {
            return -1;
        }

        // Write superblock
        if (below->write(below, 0, 0, (block_t *)&SB) < 0) {
            return -1;
        }

        // Write fatblock
        if (below->write(below, 0, get_fat_entry_block_number(fat_entry->next), (block_t *)&fat_block) < 0) {
            return -1;
        }

        return 1;
    }

    // Claim more nodes
    uint num_blocks_to_allocate = nblocks - inode_to_write->size;
    uint inode_head = inode_to_write->head == -1 ? SB.head : inode_to_write->head;

    struct fat_table_block  fat_block;
    struct entry            dummy_fat_entry = {.next = inode_head};
    struct entry            *fat_entry = &dummy_fat_entry;
    uint                    fat_block_number;

    uint sb_old_head = SB.head;
    
    // Point inode_tail to head of free list and write updated fat block to disk
    if (inode_to_write->size == 0) {
        if (below->read(below, 0, get_fat_entry_block_number(fat_entry->next), (block_t *)&fat_block) < 0) {
            return -1;
        }
        fat_entry = get_fat_entry_from_fat_table_block(fat_entry->next, &fat_block);
    } 

    fat_entry->next = SB.head;
    if (below->write(below, 0, fat_block_number, (block_t *)&fat_block) < 0) {
        return -1;
    }

    fat_walk(below, num_blocks_to_allocate, &fat_entry, &fat_block);

    // 3. Point SB.head = free[offset - size + 1].next and update superblock
    SB.head = fat_entry->next;
    if (below->write(below, 0, 0, (block_t *)&SB) < 0) {
        return -1;
    }

    // 4. Point free[offset - size + 1] = -1
    fat_entry->next = -1;
    if (below->write(below, 0, fat_block_number, (block_t *)&fat_block) < 0) {
        return -1;
    }

    // 5. Update inode size and write to disk
    inode_to_write->size = nblocks;
    if (below->write(below, 0, inode_block_number, (block_t *)&iblock) < 0) {
        return -1;
    }

    /* Replace the code below with code for changing the size of an inode. */
    /* Student's code ends here. */
}

int mydisk_create(inode_intf below, uint below_ino, uint ninodes) {
    /* Student's code goes here (File System). */

    uint nblocks = (*below->getsize)(below, below_ino);
    float fat_entry_to_block_size_ratio = BLOCK_SIZE / sizeof(struct entry);

    
    /* Initialize the on-disk data structures for your file system. */
    int iblock_count = (ninodes + INODES_PER_BLOCK - 1) / INODES_PER_BLOCK;
    int fblock_count = ((nblocks - iblock_count - 1) / (1 + fat_entry_to_block_size_ratio)) + 1;
    int remaining_block_count = nblocks - iblock_count - fblock_count - 1; // - 1 For the super block

    // Read super block
    if (below->read(below, below_ino, 0, (block_t *)&SB) < 0) {
        return -1;
    }
    
    if (SB.iblock_count == 0) {

        // Setup super block
        memset(&SB, 0, BLOCK_SIZE);


        SB.fblock_count     = fblock_count;
        SB.iblock_count     = iblock_count;
        SB.head             = 0;

        if (below->write(below, below_ino, 0, (block_t *)&SB)) {
            return -1;
        }

        // Setup inode blocks
        for (int i = 0; i < iblock_count; i++) {

            struct inode_block ib;

            for (int j = 0; j < INODES_PER_BLOCK; j++) {
                ib.inodes[j].size = 0;
                ib.inodes[j].head = -1;
            }

            // Write inode_block to disk
            if (below->write(below, below_ino, 1 + i, (block_t *)&ib)) {
                return -1;
            }

        }

        // Setup fat table blocks
        for (int i = 0; i < fblock_count; i++) {

            // Initialise a fat_table_block
            struct fat_table_block fb;

            for (int j = 0; j < FAT_ENTRIES_PER_BLOCK; j++) {
                if (remaining_block_count > 1) {
                    fb.entries[j].next = (i * FAT_ENTRIES_PER_BLOCK) + j + 1;
                    remaining_block_count--;
                } else {
                    fb.entries[j].next = -1;
                }

            }

            if (below->write(below, below_ino, 1 + iblock_count + i, (block_t *)&fb) < 0) {
                return -1;
            }
        }
        
    }

    /* Student's code ends here. */
    return 0;
}

inode_intf mydisk_init(inode_intf below, uint below_ino) {
    /* Student's code goes here (File System). */

    /* Feel free to modify anything below if necessary. */
    inode_intf self = malloc(sizeof(struct inode_store));
    self->getsize   = mydisk_getsize;
    self->setsize   = mydisk_setsize;
    self->read      = mydisk_read;
    self->write     = mydisk_write;
    self->state     = below;
    return self;
    /* Student's code ends here. */
}

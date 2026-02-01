/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: bootloader
 * Initialize the tty device, disk device, MMU, and CPU interrupts.
 */

#include "egos.h"

void tty_init();
void disk_init();
void mmu_init();
void intr_init(uint core_id);
void grass_entry(uint core_id);

extern void init_display();

struct grass* grass = (void*)GRASS_STRUCT;
struct earth* earth = (void*)EARTH_STRUCT;

static inline void vga_write_reg(uint port, uint index, uint value) {
    // Write index to port
    uint addr = VGA_BASE + port + QEMU_VGA_OFFSET;
    ACCESS((uint *)(addr)) = index;
    // Write data to port+1
    ACCESS((uint *)(addr + 1)) = value;
}

static inline uint vga_read_reg(uint port, uint index) {
    uint addr = VGA_BASE + port + QEMU_VGA_OFFSET;
    // Write index to port
    ACCESS((uint *)(addr + port)) = index;
    // Read data from port+1
    return ACCESS((uint *)(addr + 1));
}

void boot() {
    uint core_id, vendor_id;
    asm("csrr %0, mhartid" : "=r"(core_id));
    asm("csrr %0, mvendorid" : "=r"(vendor_id));
    earth->platform = (vendor_id == 666) ? HARDWARE : QEMU;

    if (booted_core_cnt++ == 0) {
        /* The first booted core needs to do some more work. */
        tty_init();
        CRITICAL("--- Booting on %s with core #%d ---",
                 earth->platform == HARDWARE ? "Hardware" : "QEMU", core_id);

        disk_init();
        SUCCESS("Finished initializing the tty and disk devices");

        earth->disk_test();
        SUCCESS("Finished testing disk");
        
        
        mmu_init();
        intr_init(core_id);
        SUCCESS("Finished initializing the MMU, timer and interrupts");
        
        /* Student's code goes here (I/O Device Driver). */
        // init_display();
        /* Student's code ends here. */

        grass_entry(core_id);
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (Multicore & Locks). */

        /* Initialize the MMU and interrupts on this CPU core.
         * Read mmu_init() and intr_init(), and decide what to do here. */

        /* Reset the timer, release the boot lock, and then hang the core
           by waiting for a timer interrupt using the wfi instruction. */

        /* Student's code ends here. */
    }
}


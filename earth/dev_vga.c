#include <stdint.h>

#include "egos.h"

#define PCI_ECAM_ALLOW_MMIO_AND_DMA ((1 << 1) | (1 << 2))
#define VGA_REMAP VGA_BASE + QEMU_VGA_OFFSET
#define WHW(index, value) (ushort)((value << 8) | index) // Write half word, used to update indexed registers with a single mmio write

void vga_init() {
    /* Student's code goes here (I/O Device Driver). */
       /* Initialize QEMU's standard VGA device for apps/user/video_demo.c.
         * Start with https://www.qemu.org/docs/master/specs/standard-vga.html,
         * and you could ask ChatGPT about the Bochs Dispi (Display Interface).
         * Your driver should setup the PCI ECAM for VGA, and then set the VGA
         * screen resolution to 800*600 pixels, each using 4 bytes for its RGB
         * information. Lastly, initialize all the pixels with white color. */

        

        REGW(VGA_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;
        REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE;
        REGW(VGA_PCI_ECAM, 0x18) = VGA_BASE;

        char *edid = (char*)VGA_BASE;
        for (uint i = 0; i < 128; i++) {
            INFO("edid[%d]: %x",i, edid[i]);
        }

        CRITICAL("Detailed Timing Descriptor");
        for (uint i = 128; i < 256; i++) {
            INFO("edid[%d]: %x",i - 128, edid[i]);
        }

        uint info = REGW(VGA_PCI_ECAM, 0x0);
        uint cmd = REGW(VGA_PCI_ECAM, 0x4);
        REGW(VGA_PCI_ECAM, 0x18) = 0xFFFFFFFF;
        uint bar2_size = REGW(VGA_PCI_ECAM, 0x18);
        bar2_size = ~(bar2_size & ~0xF) + 1;

        REGW(VGA_PCI_ECAM, 0x18) = VGA_BASE;

        INFO("VGA Device ID: %x", info & 0xFFFF0000);
        INFO("VGA Vendor ID: %x", info & 0x0000FFFF);
        INFO("Command Register: %x", cmd & 0x0000FFFF);
        INFO("BAR2 size: %x", bar2_size);

        // REGB(VGA_BASE, 0x3) = REGB(VGA_BASE, 0x3) | 0x80;
        // REGB(VGA_BASE, 0x11) = REGB(VGA_BASE, 0x11) & ~0x80;

        REGB(VGA_REMAP, 0x3C2) = 0xE3;

        INFO("Misc Output Register: %x", REGB(VGA_REMAP, 0x3CC));
        REGB(VGA_REMAP, 0x3C4) = 0x01;
        INFO("Clocking Mode Register: %x", REGB(VGA_REMAP, 0x3C5));


        // Sample Register settings mode 0x13
        // Alternate between read and writes
        volatile int read1 = REGB(VGA_REMAP, 0x3DA);
        REGB(VGA_REMAP, 0x3C0) = 0x10;
        REGB(VGA_REMAP, 0x3C0) = 0x41;

        volatile int read2 = REGB(VGA_REMAP, 0x3DA);
        REGB(VGA_REMAP, 0x3C0) = 0x11;
        REGB(VGA_REMAP, 0x3C0) = 0x00;

        volatile int read3 = REGB(VGA_REMAP, 0x3DA);
        REGB(VGA_REMAP, 0x3C0) = 0x12;
        REGB(VGA_REMAP, 0x3C0) = 0x0F;

        volatile int read4 = REGB(VGA_REMAP, 0x3DA);
        REGB(VGA_REMAP, 0x3C0) = 0x13;
        REGB(VGA_REMAP, 0x3C0) = 0x00;

        volatile int read5 = REGB(VGA_REMAP, 0x3DA);
        REGB(VGA_REMAP, 0x3C0) = 0x14;
        REGB(VGA_REMAP, 0x3C0) = 0x00;

        volatile int read6 = REGB(VGA_REMAP, 0x3DA);

        REGB(VGA_REMAP, 0x3C2) = 0x63;

        REGHW(VGA_REMAP, 0x3C4) = WHW(0x01, 0x01);
        // REGHW(VGA_REMAP, 0x3C4) = WHW(0x02, 0x0F);
        REGHW(VGA_REMAP, 0x3C4) = WHW(0x03, 0x00);
        REGHW(VGA_REMAP, 0x3C4) = WHW(0x04, 0x0E);

        REGHW(VGA_REMAP, 0x3CE) = WHW(0x05, 0x40);
        REGHW(VGA_REMAP, 0x3CE) = WHW(0x06, 0x05);

        REGHW(VGA_REMAP, 0x3D4) = WHW(0x00, 0x5F);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x01, 0x4F);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x02, 0x50);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x03, 0x82);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x04, 0x54);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x05, 0x80);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x06, 0xBF);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x07, 0x1F);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x08, 0x00);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x09, 0x41);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x10, 0x9C);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x11, 0x8E);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x12, 0x8F);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x13, 0x28);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x14, 0x40);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x15, 0x96);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x16, 0xB9);
        REGHW(VGA_REMAP, 0x3D4) = WHW(0x17, 0xA3);

        REGB(VGA_REMAP, 0x3D4) = 0x00;
        INFO("horizontal_total: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x01;
        INFO("horizontal_display_end: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x02;
        INFO("horizontal_blanking_start: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x03;
        INFO("horizontal_blanking_end: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x04;
        INFO("horizontal_sync_start: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x05;
        INFO("horizontal_sync_end: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x06;
        INFO("vertical_total: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x10;
        INFO("vertical_display_end: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x11;
        INFO("vertical_blanking_start: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x12;
        INFO("vertical_blanking_end: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x15;
        INFO("vertical_sync_start: %d", REGB(VGA_REMAP, 0x3D5));

        REGB(VGA_REMAP, 0x3D4) = 0x16;
        INFO("vertical_sync_end: %d", REGB(VGA_REMAP, 0x3D5));

#define HRES 800
#define VRES 600

        for (uint i = 0; i < VRES; i++) {
            for (uint j = 0; j < HRES; j++) {
                /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
                uint* pix = (uint*)(VIDEO_FRAME_BASE + 4 * (i * HRES + (HRES - j)));

                /* Only 3 bytes are used for RGB and the 4th byte is unused. */
                *pix = 0x00FFFFFF;
            }
        }

        // for (uint i = 0; i < VRES; i++) {
        //     for (uint j = 0; j < HRES; j++) {
        //         /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
        //         uint* pix = (uint*)(VIDEO_FRAME_BASE + 4 * (i * HRES + (HRES - j)));

        //         /* Only 3 bytes are used for RGB and the 4th byte is unused. */
        //         INFO("pixel in row %d, col %d has value: %x", i, j, pix);
        //     }
        // }

        SUCCESS("Finished initialising the VGA device");
        /* Student's code ends here. */
}

// Registers
#define BOCHS_OFFSET 0x0500

#define VBE_DISPI_INDEX_ID                  (0x00 << 1) + VGA_BASE + BOCHS_OFFSET   // 0
#define VBE_DISPI_INDEX_XRES                (0x01 << 1) + VGA_BASE + BOCHS_OFFSET   // 1
#define VBE_DISPI_INDEX_YRES                (0x02 << 1) + VGA_BASE + BOCHS_OFFSET   // 2
#define VBE_DISPI_INDEX_BPP                 (0x03 << 1) + VGA_BASE + BOCHS_OFFSET   // 3
#define VBE_DISPI_INDEX_ENABLE              (0x04 << 1) + VGA_BASE + BOCHS_OFFSET   // 4
#define VBE_DISPI_INDEX_BANK                (0x05 << 1) + VGA_BASE + BOCHS_OFFSET   // 5
#define VBE_DISPI_INDEX_VIRT_WIDTH          (0x06 << 1) + VGA_BASE + BOCHS_OFFSET   // 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT         (0x07 << 1) + VGA_BASE + BOCHS_OFFSET   // 7
#define VBE_DISPI_INDEX_X_OFFSET            (0x08 << 1) + VGA_BASE + BOCHS_OFFSET   // 8
#define VBE_DISPI_INDEX_Y_OFFSET            (0x09 << 1) + VGA_BASE + BOCHS_OFFSET   // 9

// Values
#define VBE_DISPI_DISABLED      0x00
#define VBE_DISPI_ENABLED       0x01
#define VBE_DISPI_BPP_24        0x18
#define VBE_DISPI_BPP_32        0x20
#define VBE_DISPI_LFB_ENABLED   0x40

void bochs_vga_init() {
    REGW(VGA_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;

    CRITICAL("Orginal BAR0: %x", REGW(VGA_PCI_ECAM, 0x10));
    CRITICAL("Orginal BAR2: %x", REGW(VGA_PCI_ECAM, 0x18));

    REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE;
    REGW(VGA_PCI_ECAM, 0x18) = VGA_BASE;

    CRITICAL("Updated BAR0: %x", REGW(VGA_PCI_ECAM, 0x10));
    CRITICAL("Updated BAR2: %x", REGW(VGA_PCI_ECAM, 0x18));

    uint id = REGHW(VBE_DISPI_INDEX_ID, 0x00);

    CRITICAL("BGA: %x", id);

    REGHW(VBE_DISPI_INDEX_ENABLE, 0x00) = VBE_DISPI_DISABLED;
    REGHW(VBE_DISPI_INDEX_XRES, 0x00) = 800;
    REGHW(VBE_DISPI_INDEX_YRES, 0x00) = 600;
    REGHW(VBE_DISPI_INDEX_BPP, 0x00) = VBE_DISPI_BPP_32;

    for (uint i = 0; i < VRES; i++) {
        for (uint j = 0; j < HRES; j++) {
            /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
            uint32_t* pix = (uint32_t*)(VIDEO_FRAME_BASE + 4 * (i * 800 + j));

            /* Only 3 bytes are used for RGB and the 4th byte is unused. */
            *pix = 0x00FFFFFF;
        }
    }

    REGHW(VBE_DISPI_INDEX_ENABLE, 0x00) = REGHW(VBE_DISPI_INDEX_ENABLE, 0x00) | VBE_DISPI_INDEX_ENABLE;

    SUCCESS("Finished initialising vga device");
}

#define VIRTIO_MAGIC      0x000 + 
#define VIRTIO_VERSION    0x004 + 
#define VIRTIO_DEVICE_ID  0x008 + 
#define VIRTIO_VENDOR_ID  0x00c + 
#define VIRTIO_STATUS     0x070 + 

void some_vga_function() {
    CRITICAL("some_vga_function()");
    /* Student's code goes here (I/O Device Driver). */
       /* Initialize QEMU's standard VGA device for apps/user/video_demo.c.
         * Start with https://www.qemu.org/docs/master/specs/standard-vga.html,
         * and you could ask ChatGPT about the Bochs Dispi (Display Interface).
         * Your driver should setup the PCI ECAM for VGA, and then set the VGA
         * screen resolution to 800*600 pixels, each using 4 bytes for its RGB
         * information. Lastly, initialize all the pixels with white color. */

        
        REGW(VGA_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;

        CRITICAL("Orginal BAR0: %x", REGW(VGA_PCI_ECAM, 0x10));
        CRITICAL("Orginal BAR2: %x", REGW(VGA_PCI_ECAM, 0x18));

        REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE;
        REGW(VGA_PCI_ECAM, 0x18) = VGA_BASE;

        CRITICAL("Updated BAR0: %x", REGW(VGA_PCI_ECAM, 0x10));
        CRITICAL("Updated BAR2: %x", REGW(VGA_PCI_ECAM, 0x18));

        REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE;
        REGW(VGA_PCI_ECAM, 0x18) = VGA_BASE;
}

void ramfb_init() {
    int width = 800;
    int height = 600;
    uint *fb = (uint *)VIDEO_FRAME_BASE;

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            fb[y * width + x] = 0x00FF0000; // solid red (BGRX)
}

enum VGA_PORT {
    PORT_0x3C0  =   0x3C0,
    PORT_0x3C2  =   0x3C2,
    PORT_0x3C4  =   0x3C4,
    PORT_0x3CE  =   0x3CE,
    PORT_0x3D4  =   0x3D4,
};

typedef struct {
    uint16_t           port;
    uint8_t            index;
    uint8_t            data;
} vga_tuple;

static const vga_tuple vga_mode_13_sequence[] = {
    {0x3C2, 0x00, 0x63},
    {0x3D4, 0x11, 0x0E},
    {0x3D4, 0x00, 0x5F},
    {0x3D4, 0x01, 0x4F},
    {0x3D4, 0x02, 0x50},
    {0x3D4, 0x03, 0x82},
    {0x3D4, 0x04, 0x54},
    {0x3D4, 0x05, 0x80},
    {0x3D4, 0x06, 0xBF},
    {0x3D4, 0x07, 0x1F},
    {0x3D4, 0x08, 0x00},
    {0x3D4, 0x09, 0x41},
    {0x3D4, 0x10, 0x9C},
    {0x3D4, 0x11, 0x8E},
    {0x3D4, 0x12, 0x8F},
    {0x3D4, 0x13, 0x28},
    {0x3D4, 0x14, 0x40},
    {0x3D4, 0x15, 0x96},
    {0x3D4, 0x16, 0xB9},
    {0x3D4, 0x17, 0xA3},
    {0x3C4, 0x01, 0x01},
    {0x3C4, 0x02, 0x0F},
    {0x3C4, 0x04, 0x0E},
    {0x3CE, 0x00, 0x00},
    {0x3CE, 0x05, 0x40},
    {0x3CE, 0x06, 0x05},
    {0x3C0, 0x30, 0x41},
    {0x3C0, 0x33, 0x00},
};

static const int PALETTE[256] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA, 0x555555,
    0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF, 0x000000, 0x330000,
    0x660000, 0x990000, 0xCC0000, 0xFF0000, 0x003300, 0x333300, 0x663300, 0x993300, 0xCC3300,
    0xFF3300, 0x006600, 0x336600, 0x666600, 0x996600, 0xCC6600, 0xFF6600, 0x009900, 0x339900,
    0x669900, 0x999900, 0xCC9900, 0xFF9900, 0x00CC00, 0x33CC00, 0x66CC00, 0x99CC00, 0xCCCC00,
    0xFFCC00, 0x00FF00, 0x33FF00, 0x66FF00, 0x99FF00, 0xCCFF00, 0xFFFF00, 0x000033, 0x330033,
    0x660033, 0x990033, 0xCC0033, 0xFF0033, 0x003333, 0x333333, 0x663333, 0x993333, 0xCC3333,
    0xFF3333, 0x006633, 0x336633, 0x666633, 0x996633, 0xCC6633, 0xFF6633, 0x009933, 0x339933,
    0x669933, 0x999933, 0xCC9933, 0xFF9933, 0x00CC33, 0x33CC33, 0x66CC33, 0x99CC33, 0xCCCC33,
    0xFFCC33, 0x00FF33, 0x33FF33, 0x66FF33, 0x99FF33, 0xCCFF33, 0xFFFF33, 0x000066, 0x330066,
    0x660066, 0x990066, 0xCC0066, 0xFF0066, 0x003366, 0x333366, 0x663366, 0x993366, 0xCC3366,
    0xFF3366, 0x006666, 0x336666, 0x666666, 0x996666, 0xCC6666, 0xFF6666, 0x009966, 0x339966,
    0x669966, 0x999966, 0xCC9966, 0xFF9966, 0x00CC66, 0x33CC66, 0x66CC66, 0x99CC66, 0xCCCC66,
    0xFFCC66, 0x00FF66, 0x33FF66, 0x66FF66, 0x99FF66, 0xCCFF66, 0xFFFF66, 0x000099, 0x330099,
    0x660099, 0x990099, 0xCC0099, 0xFF0099, 0x003399, 0x333399, 0x663399, 0x993399, 0xCC3399,
    0xFF3399, 0x006699, 0x336699, 0x666699, 0x996699, 0xCC6699, 0xFF6699, 0x009999, 0x339999,
    0x669999, 0x999999, 0xCC9999, 0xFF9999, 0x00CC99, 0x33CC99, 0x66CC99, 0x99CC99, 0xCCCC99,
    0xFFCC99, 0x00FF99, 0x33FF99, 0x66FF99, 0x99FF99, 0xCCFF99, 0xFFFF99, 0x0000CC, 0x3300CC,
    0x6600CC, 0x9900CC, 0xCC00CC, 0xFF00CC, 0x0033CC, 0x3333CC, 0x6633CC, 0x9933CC, 0xCC33CC,
    0xFF33CC, 0x0066CC, 0x3366CC, 0x6666CC, 0x9966CC, 0xCC66CC, 0xFF66CC, 0x0099CC, 0x3399CC,
    0x6699CC, 0x9999CC, 0xCC99CC, 0xFF99CC, 0x00CCCC, 0x33CCCC, 0x66CCCC, 0x99CCCC, 0xCCCCCC,
    0xFFCCCC, 0x00FFCC, 0x33FFCC, 0x66FFCC, 0x99FFCC, 0xCCFFCC, 0xFFFFCC, 0x0000FF, 0x3300FF,
    0x6600FF, 0x9900FF, 0xCC00FF, 0xFF00FF, 0x0033FF, 0x3333FF, 0x6633FF, 0x9933FF, 0xCC33FF,
    0xFF33FF, 0x0066FF, 0x3366FF, 0x6666FF, 0x9966FF, 0xCC66FF, 0xFF66FF, 0x0099FF, 0x3399FF,
    0x6699FF, 0x9999FF, 0xCC99FF, 0xFF99FF, 0x00CCFF, 0x33CCFF, 0x66CCFF, 0x99CCFF, 0xCCCCFF,
    0xFFCCFF, 0x00FFFF, 0x33FFFF, 0x66FFFF, 0x99FFFF, 0xCCFFFF, 0xFFFFFF, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void set_mode13(uint io_base) {
    // Set Mode 13
    vga_tuple curr;
    for (int i = 0; i < sizeof(vga_mode_13_sequence); i++) {
        curr = vga_mode_13_sequence[i];
        if (curr.port == 0x3C0) {
            REGB(io_base, 0x3DA) = 0;
            REGB(io_base, curr.port) = curr.index;
            REGB(io_base, curr.port) = curr.data;
        } else if (curr.port == 0x3C2) {
            REGB(io_base, curr.port) = curr.data;
        } else {
            REGB(io_base, curr.port) = curr.index;
            REGB(io_base, curr.port + 0x01) = curr.data;
        }
    }

    REGB(io_base, 0x406) = 0xFF;
    REGB(io_base, 0x408) = 0;
    uint p = io_base + 0x409;

    uint8_t r, g, b;

    for (int i = 0; i < sizeof(PALETTE); i++) {
        b = PALETTE[i] & 0x0000FF;
        g = (PALETTE[i] >> 8) & 0x00FF;
        r = (PALETTE[i] >> 16) * 0xFF;

        REGB(p, 0x00) = r;
        REGB(p, 0x00) = g;
        REGB(p, 0x00) = b;
    }
}

void vga_retry() {
    REGW(VGA_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;
    
    uint info = REGW(VGA_PCI_ECAM, 0x0);
    uint cmd = REGW(VGA_PCI_ECAM, 0x4);
    uint class_info = REGW(VGA_PCI_ECAM, 0x8);

    REGW(VGA_PCI_ECAM, 0x10) = 0xFFFFFFFF;
    uint fb_size = REGW(VGA_PCI_ECAM, 0x10);
    fb_size = ~(fb_size | 0xF) + 1;

    uint io_base = VIDEO_FRAME_BASE + fb_size;

    REGW(VGA_PCI_ECAM, 0x18) = 0xFFFFFFFF;
    uint io_size = REGW(VGA_PCI_ECAM, 0x18);
    io_size = ~(io_size | 0xF) + 1;

    REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE | 8;
    REGW(VGA_PCI_ECAM, 0x18) = io_base | 8;

    REGB(VGA_PCI_ECAM, 0x04) = (REGB(VGA_PCI_ECAM, 0x04) | 0x0002);

    INFO("VGA fb_base %x, size %x, io_base %x, size: %x", VIDEO_FRAME_BASE, fb_size, io_base, io_size);
    
    INFO("VGA Device ID: %x", info & 0xFFFF0000);
    INFO("VGA Vendor ID: %x", info & 0x0000FFFF);
    INFO("Command Register: %x", cmd & 0x0000FFFF);
    INFO("Status: %x", cmd >> 16);
    INFO("Class: %x", class_info);
    INFO("io_size: %x", io_size);

    set_mode13(io_base);

    REGB(io_base, 0x409) = 1;

    for (int i = 0; i < 320 * 200; i++) {
        REGB(VIDEO_FRAME_BASE, 0x01 * i) = 0;
    } 
    
    if (class_info >> 24 != 0x03) {
        FATAL("virtio-vga was not found");
    }
}

void vga_simple() {
    #define VGA_FB ((volatile uint32_t*)0x10000000)

    int x = 100;
    int y = 50;
    VGA_FB[y * 640 + x] = 0x00FF0000;
}

#define WIDTH 320
#define HEIGHT 200
#define PIXEL 0x00FF0000 // red

// Simplified Virtio GPU structures
struct virtio_gpu_ctrl_hdr {
    uint32_t type;
    uint32_t flags;
    uint64_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
};

struct virtio_gpu_resource_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t format;
    uint32_t width;
    uint32_t height;
};

// Fake MMIO/virtqueue writes for example purposes
volatile uint32_t *gpu_regs = (volatile uint32_t *)0x10000000; // map BAR0 here

// Minimal virtio-gpu sequence
void init_display() {
    uint32_t resource_id = 1;

    // 1. Create resource
    struct virtio_gpu_resource_2d res = {
        .hdr = {.type = 0x0100}, // VIRTIO_GPU_CMD_RESOURCE_CREATE_2D
        .resource_id = resource_id,
        .format = 1, // XRGB8888
        .width = WIDTH,
        .height = HEIGHT
    };
    // write to control virtqueue (simplified)
    gpu_regs[0] = *((uint32_t *)&res);

    // 2. Attach to scanout
    struct virtio_gpu_ctrl_hdr scanout = {
        .type = 0x0200, // VIRTIO_GPU_CMD_SET_SCANOUT
        .flags = 0,
        .fence_id = 0,
        .ctx_id = 0,
        .padding = 0
    };
    gpu_regs[0] = *((uint32_t *)&scanout);

    // 3. Fill framebuffer
    volatile uint32_t *fb = gpu_regs + 0x1000; // BAR0 framebuffer
    for (int i = 0; i < WIDTH*HEIGHT; i++)
        fb[i] = PIXEL;

    // 4. Flush
    struct virtio_gpu_ctrl_hdr flush = {.type = 0x0400};
    gpu_regs[0] = *((uint32_t *)&flush);
}
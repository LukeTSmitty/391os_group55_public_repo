#include <string.h>
#include <assert.h>
#include "conf.h"
#include "console.h"
#include "elf.h"
#include "assert.h"
#include "thread.h"
#include "process.h"
#include "memory.h"
#include "fs.h"
#include "io.h"
#include "device.h"
#include "dev/rtc.h"
#include "dev/uart.h"
#include "intr.h"
#include "dev/virtio.h"
#include "heap.h"
#include "string.h"

#define BLOCK_SIZE 512
#define NUM_DIRECT 3
#define NUM_INDIR 128

#define VIRTIO_MMIO_STEP (VIRTIO1_MMIO_BASE-VIRTIO0_MMIO_BASE)
extern char _kimg_end[]; 

int main() {
    struct io *blkio;
    int result;
    console_init();
    devmgr_init();
    intrmgr_init();
    thrmgr_init();
    memory_init();
    procmgr_init();

    uart_attach((void*)UART0_MMIO_BASE, UART0_INTR_SRCNO+0);
    uart_attach((void*)UART1_MMIO_BASE, UART0_INTR_SRCNO+1);
    rtc_attach((void*)RTC_MMIO_BASE);
    
    for (int i = 0; i < 8; i++) {
        virtio_attach ((void*)VIRTIO0_MMIO_BASE + i*VIRTIO_MMIO_STEP, VIRTIO0_INTR_SRCNO + i);
    }
    
    result = open_device("vioblk", 0, &blkio);
    if (result < 0) {
        kprintf("Error: %d\n", result);
        panic("Failed to open vioblk\n");
    }

    result = fsmount(blkio);
    if (result < 0) {
        kprintf("Error: %d\n", result);
        panic("Failed to mount filesystem\n");
    }

    // const char *filename = "testfile.dat";
    // const char *data = "Hello, KTFS!";
    // const char *data2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // char read_buf[64] = {0};
    // struct io *file_io = NULL;
    // unsigned long long *end = kcalloc(1, sizeof(unsigned long long));

    // Create and open file
    // assert(fscreate(filename) == 0);
    // assert(fsopen(filename, &file_io) == 0);

    // *end = 0 + strlen(data);
    // const char *filename = "dsave.dat\0";
    // // const char *filename2 = "dtextc.dat\0";
    // const char *data = "Hello, KTFS!";
    // // const char *data2= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // char read_buf[64] = {0};
    // struct io *file_io = NULL;
    // unsigned long long * end = kcalloc(1, sizeof(unsigned long long *));

    // // Create file
    // // assert(fsdelete(filename) == 0);

    // assert(fscreate(filename) == 0);

    // // Open file
    // // struct io *file_io2 = NULL;
    // // assert(fsopen(filename2, &file_io2) == 0);
    // // assert(file_io2 != NULL);

    // assert(fsopen(filename, &file_io) == 0);
    // assert(file_io != NULL);

    // // // Extend the end of the file to just say hello
    // *end = 5;
    // assert(ioctl(file_io, IOCTL_SETEND, end) == 0);
    // assert(iowriteat(file_io, 0, data, *end) == *end);
    // assert(ioreadat(file_io, 0, read_buf, *end) == *end);
    // assert(memcmp(read_buf, data, *end) == 0);

    // kprintf("Expected: '%s'\n", data);
    // kprintf("Actual: '%s'\n", read_buf);
    // kprintf("Write/Read at start with truncation\n");



    // *end = strlen(data);
    // assert(ioctl(file_io, IOCTL_SETEND, end) == 0);
    // assert(iowriteat(file_io, 0, data, strlen(data)) == (long)strlen(data));
    // assert(ioreadat(file_io, 0, read_buf, strlen(data)) == (long)strlen(data));
    // assert(memcmp(read_buf, data, strlen(data)) == 0);
    // kprintf("Direct block write/read passed\n");

    // size_t offset_indirect = BLOCK_SIZE * NUM_DIRECT;
    // *end = offset_indirect + strlen(data2);
    // assert(ioctl(file_io, IOCTL_SETEND, end) == 0);
    // assert(iowriteat(file_io, offset_indirect, data2, strlen(data2)) == (long)strlen(data2));
    // memset(read_buf, 0, sizeof(read_buf));
    // assert(ioreadat(file_io, offset_indirect, read_buf, strlen(data2)) == (long)strlen(data2));
    // assert(memcmp(read_buf, data2, strlen(data2)) == 0);
    // kprintf("Indirect block write/read passed\n");

    // size_t offset_dindirect = BLOCK_SIZE * (NUM_DIRECT + NUM_INDIR);
    // *end = offset_dindirect + strlen(data2);
    // assert(ioctl(file_io, IOCTL_SETEND, end) == 0);
    // assert(iowriteat(file_io, offset_dindirect, data2, strlen(data2)) == (long)strlen(data2));
    // memset(read_buf, 0, sizeof(read_buf));
    // assert(ioreadat(file_io, offset_dindirect, read_buf, strlen(data2)) == (long)strlen(data2));
    // assert(memcmp(read_buf, data2, strlen(data2)) == 0);
    // kprintf("Doubly indirect block write/read passed\n");

    // ioclose(file_io);

    // assert(fsopen(filename, &file_io) == 0);

    // assert(ioreadat(file_io, 0, read_buf, strlen(data)) == (long)strlen(data));
    // assert(memcmp(read_buf, data, strlen(data)) == 0);
    // kprintf("Re-read direct block passed\n");

    // assert(ioreadat(file_io, offset_indirect, read_buf, strlen(data2)) == (long)strlen(data2));
    // assert(memcmp(read_buf, data2, strlen(data2)) == 0);
    // kprintf("Re-read indirect block passed\n");

    // assert(ioreadat(file_io, offset_dindirect, read_buf, strlen(data2)) == (long)strlen(data2));
    // assert(memcmp(read_buf, data2, strlen(data2)) == 0);
    // kprintf("Re-read doubly indirect block passed\n");

    // ioclose(file_io);

    // kfree(end);

    char data[] = "KTFS Positioning Test String!!";
    unsigned long long *pos = kcalloc(1, sizeof(unsigned long long));
    unsigned long long *end = kcalloc(1, sizeof(unsigned long long));
    struct io *file_io = NULL;

    // Open or create the file
    fscreate("new.dat");
    assert(fsopen("new.dat", &file_io) == 0);

    // Write to the file
    size_t len = strlen(data);
    *end = len;
    assert(ioctl(file_io, IOCTL_SETEND, end) == 0);
    assert(iowriteat(file_io, 0, data, len) == len);

    // Check GETEND after write
    assert(ioctl(file_io, IOCTL_GETEND, end) == 0);
    kprintf("GETEND returned: %llu (expected: %zu)\n", *end, len);
    assert(*end == len);

    // Move file position using SETPOS
    *pos = 10;
    assert(ioctl(file_io, IOCTL_SETPOS, pos) == 0);

    // Verify with GETPOS
    assert(ioctl(file_io, IOCTL_GETPOS, pos) == 0);
    kprintf("GETPOS returned: %llu (expected: 10)\n", *pos);
    assert(*pos == 10);

    // Read from current position (should be "ositioning...")
    char read_buf[32] = {0};
    assert(ioread(file_io, read_buf, 20) == 20);
    kprintf("Read after SETPOS: '%s'\n", read_buf);

    // Get updated position after read
    assert(ioctl(file_io, IOCTL_GETPOS, pos) == 0);
    kprintf("GETPOS after read: %llu (expected: 30)\n", *pos);
    assert(*pos == 30); // 10 + 20

    ioclose(file_io);

    assert(fsopen("new.dat", &file_io) == 0);

    // Verify end is persisted
    assert(ioctl(file_io, IOCTL_GETEND, end) == 0);
    kprintf("GETEND after reopen: %llu (expected: %zu)\n", *end, len);
    assert(*end == len);

    // Verify position resets to 0
    assert(ioctl(file_io, IOCTL_GETPOS, pos) == 0);
    kprintf("GETPOS after reopen: %llu (expected: 0)\n", *pos);
    assert(*pos == 0);

    kfree(pos);
    kfree(end);

    fsdelete("new.dat");
    kprintf("IOCNTL passed\n");
    return 0;
}

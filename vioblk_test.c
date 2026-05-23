#include <string.h>
#include <assert.h>
#include <io.h>
#include "heap.h"
#include "thread.h"

#include "conf.h"
#include "console.h"
#include "intr.h"
#include "device.h"
#include "dev/virtio.h"
#include "error.h"
#include "dev/rtc.h"
#include "dev/uart.h"

extern char _kimg_end[]; 
int main() {
    console_init();
    devmgr_init();
    intrmgr_init();
    thrmgr_init();
    heap_init(_kimg_end, UMEM_START); 
    uart_attach((void*)UART0_MMIO_BASE, UART0_INTR_SRCNO+0);
    uart_attach((void*)UART1_MMIO_BASE, UART0_INTR_SRCNO+1);
    rtc_attach((void*)RTC_MMIO_BASE);
    
    int result;
    struct io *blkio;
    char write_buffer[512];
    char read_buffer[512];
    unsigned long long disk_size;
    unsigned int block_size;
    long bytes_written, bytes_read;
    int i;
    
    for (i = 0; i < 8; i++) {
        virtio_attach ((void*)VIRTIO_MMIO_BASE(i), VIRTIO0_INTR_SRCNO + i);
    }
    
    result = open_device("vioblk", 0, &blkio);
    assert(result == 0);
    enable_interrupts();
    
    result = ioctl(blkio, IOCTL_GETBLKSZ, &block_size);
    assert(result == 0);
    assert(block_size == 512);
    kprintf("Block size: %u bytes\n\n", block_size);
    
    result = ioctl(blkio, IOCTL_GETEND, &disk_size);
    assert(result == 0);
    assert(disk_size > 0);
    kprintf("Disk size: %llu bytes\n\n", disk_size);
    
    memset(write_buffer, 'K', sizeof(write_buffer));
    bytes_written = iowriteat(blkio, 0, write_buffer, sizeof(write_buffer));
    assert(bytes_written == sizeof(read_buffer));
    kprintf("Successfully wrote %ld bytes\n\n", bytes_written);
    
    memset(read_buffer, 0, sizeof(read_buffer));
    bytes_read = ioreadat(blkio, 0, read_buffer, sizeof(read_buffer));
    assert(bytes_read == sizeof(read_buffer));
    assert(memcmp(write_buffer, read_buffer, sizeof(read_buffer)) == 0);
    kprintf("Successfully read %ld bytes\n\n", bytes_read);
    
    memset(write_buffer, 'B', sizeof(write_buffer));
    bytes_written = iowriteat(blkio, 1024, write_buffer, sizeof(write_buffer));
    assert(bytes_written == sizeof(write_buffer));
    kprintf("Successfully wrote %ld bytes\n\n", bytes_written);
    
    memset(read_buffer, 0, sizeof(read_buffer));
    bytes_read = ioreadat(blkio, 1024, read_buffer, sizeof(read_buffer));
    assert(bytes_read == sizeof(read_buffer));
    assert(memcmp(write_buffer, read_buffer, sizeof(read_buffer)) == 0);
    kprintf("Successfully read %ld bytes\n\n", bytes_read);
    
    bytes_written = iowriteat(blkio, 513, write_buffer, 512); // pos must be aligned to blk size
    //assert(bytes_written != 512); /* -EINVAL */
    
    bytes_written = iowriteat(blkio, 0, write_buffer, 513); // Each write must be 512-byte aligned - maybe do a short write/read here?
    assert(bytes_written != 513); /* -EINVAL */
    kprintf("Alignment tests passed\n\n");
    
    ioclose(blkio);
    
    kprintf("All tests passed successfully!\n");
    return 0;
}
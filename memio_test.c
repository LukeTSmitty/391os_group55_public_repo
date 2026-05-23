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

struct io *create_memory_io(void *buf, size_t size);

void test_memio_cntl() {
    char buffer[100];
    struct io *mem_io = create_memory_io(buffer, sizeof(buffer));
    
    int block_size = ioctl(mem_io, IOCTL_GETBLKSZ, NULL);
    assert(block_size == 1);
    
    size_t end_size;
    assert(ioctl(mem_io, IOCTL_GETEND, &end_size) == 0);
    assert(end_size == sizeof(buffer));
    
    size_t new_size = 50;
    assert(ioctl(mem_io, IOCTL_SETEND, &new_size) == 0);
    
    size_t invalid_size = 150;
    assert(ioctl(mem_io, IOCTL_SETEND, &invalid_size) != 0);
    kprintf("Cntl tests passed!\n");
}

void test_memio_readat() {
    char buffer[100] = "Hello, memory I/O!";
    struct io *mem_io = create_memory_io(buffer, sizeof(buffer));
    
    char read_buf[20];
    long bytes_read = ioreadat(mem_io, 0, read_buf, sizeof(read_buf));
    assert(bytes_read == 20);
    assert(strncmp(read_buf, "Hello, memory I/O!", bytes_read) == 0);
    
    bytes_read = ioreadat(mem_io, 150, read_buf, sizeof(read_buf));
    assert(bytes_read == sizeof(read_buf));
    kprintf("Read tests passed!\n");
}

void test_memio_writeat() {
    char buffer[100] = {0};
    struct io *mem_io = create_memory_io(buffer, sizeof(buffer));
    
    const char *data = "Write test";
    long bytes_written = iowriteat(mem_io, 10, data, strlen(data));
    assert(bytes_written == (long)strlen(data));
    assert(strncmp(buffer + 10, data, strlen(data)) == 0);
    
    bytes_written = iowriteat(mem_io, 150, data, strlen(data));
    assert(bytes_written == -EINVAL);
    kprintf("Write tests passed!\n");
}

int main() {
    extern char _kimg_end[];
    heap_init(_kimg_end, UMEM_START);
    thrmgr_init();

    test_memio_cntl();
    test_memio_readat();
    test_memio_writeat();
    
    kprintf("All tests passed!\n");
    return 0;
}
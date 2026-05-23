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
#include "cache.h"

static void thread1(void *arg);
static void thread2(void *arg);

char buffer[1024] = {0};
char write_buffer[512];
char blk1[512];
char blk2[512];

int main() {
    extern char _kimg_end[];
    heap_init(_kimg_end, UMEM_START);
    thrmgr_init();
    void * cached_blk;
    char read_buffer[512];
    struct cache * mem_cache;
    struct io *mem_io = create_memory_io(buffer, sizeof(buffer));
    
    long bytes_written;
    
    memset(write_buffer, 'K', sizeof(write_buffer));
    bytes_written = iowriteat(mem_io, 0, write_buffer, sizeof(write_buffer));
    assert(bytes_written == sizeof(write_buffer));
    
    create_cache(mem_io, &mem_cache);
    kprintf("0. Initialized cache\n");

    cache_get_block(mem_cache, 0, (void **)&cached_blk);
    cache_get_block(mem_cache, 0, (void **)&cached_blk);
    assert(memcmp(write_buffer, cached_blk, sizeof(write_buffer)) == 0); 
    cache_release_block(mem_cache, cached_blk, CACHE_CLEAN);
    kprintf("1. Read from backing io into cache\n");

    void * cached_blk2;
    cache_get_block(mem_cache, 0, (void **)&cached_blk2);
    assert(cached_blk == cached_blk2);
    assert(memcmp(write_buffer, cached_blk2, sizeof(write_buffer)) == 0); 
    kprintf("2. Read from cache with single block\n");

    memset(cached_blk2, 'B', sizeof(char));
    memcpy(blk1, cached_blk2, 512);
    cache_release_block(mem_cache, cached_blk2, CACHE_DIRTY);
    void * cached_blk3;
    cache_get_block(mem_cache, 0, (void **)&cached_blk3);
    assert(memcmp(cached_blk3, blk1, sizeof(blk1)) == 0);
    cache_release_block(mem_cache, cached_blk3, CACHE_CLEAN);
    kprintf("3. Dirty Block Released from cache\n");
 
    int trektid, r30tid;
    trektid = thread_spawn("thrd1", (void *)thread1, (void*)mem_cache);
    assert (0 < trektid);

    r30tid = thread_spawn("thrd2", (void *)thread2, (void*)mem_cache);
    assert (0 < r30tid);

    memset(write_buffer, 'C', sizeof(write_buffer));
    bytes_written = iowriteat(mem_io, 512, write_buffer, sizeof(write_buffer));
    assert(bytes_written == sizeof(write_buffer));
    
    thread_join(0);
    
    void * cached_blk4;
    cache_get_block(mem_cache, 512, (void **)&cached_blk4);
    assert(memcmp(cached_blk4, blk2, sizeof(blk2)) == 0);
    cache_release_block(mem_cache, cached_blk3, CACHE_CLEAN);
    kprintf("4. Exlusive access to block test\n");

    cache_flush(mem_cache);
    kprintf("Cache flushed\n");
    bytes_written = ioreadat(mem_io, 0, read_buffer, sizeof(read_buffer));
    assert(bytes_written == sizeof(read_buffer));
    assert(memcmp(read_buffer, blk1, sizeof(blk1)) == 0);
    bytes_written = ioreadat(mem_io, 512, read_buffer, sizeof(read_buffer));
    assert(bytes_written == sizeof(read_buffer));
    assert(memcmp(read_buffer, blk2, sizeof(blk2)) == 0);

    kprintf("5. Write back test passed\n");

    kprintf("All tests passed!\n");
    return 0;
}


void thread1(void *arg) {
    struct cache *mem_cache = (struct cache *)arg;
    void * cached_blk;
    cache_get_block(mem_cache, 512, (void **)&cached_blk);
    assert(memcmp(write_buffer, cached_blk, sizeof(write_buffer)) == 0); 
    thread_yield(); // switches to thread2
    memset(cached_blk, 'A', sizeof(char));
    memcpy(blk2, cached_blk, sizeof(blk2));
    cache_release_block(mem_cache, cached_blk, CACHE_DIRTY);
}

void thread2(void *arg) {
    struct cache *mem_cache = (struct cache *)arg;
    void * cached_blk;
    cache_get_block(mem_cache, 512, (void **)&cached_blk); // wait on lock to release and switches back to thrd1
    assert(memcmp(blk2, cached_blk, sizeof(blk2)) == 0); 
    memset(cached_blk + 1, 'B', sizeof(char));
    memcpy(blk2, cached_blk, sizeof(blk2));
    cache_release_block(mem_cache, cached_blk, CACHE_DIRTY);
}
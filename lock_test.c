
#include "conf.h"
#include "assert.h"
#include "console.h"
#include "intr.h"
#include "device.h"
#include "thread.h"
#include "heap.h"
#include "dev/virtio.h"
#include "dev/uart.h"
#include "timer.h"
#include <string.h>

#define TP ((struct thread*)__builtin_thread_pointer())

static void thread1(void);
static void thread2(void);
struct lock test_lock;
struct lock test_lock_copy;

void main(void) {
    extern char _kimg_end[];
    int trektid, r30tid;
    int i;
    
    console_init();
    intrmgr_init();
    timer_init();
    devmgr_init();
    thrmgr_init();

    heap_init(_kimg_end, RAM_END);
    for (i = 0; i < 3; i++)
        uart_attach((void*)UART_MMIO_BASE(i), UART0_INTR_SRCNO+i);

    for (i = 0; i < 8; i++)
        virtio_attach((void*)VIRTIO_MMIO_BASE(i), VIRTIO0_INTR_SRCNO+i);

    enable_interrupts();

    kprintf("Test 1: Acquiring Single Lock\n");

    lock_init(&test_lock);

    lock_acquire(&test_lock);
    assert(test_lock.owner == TP);
    assert(test_lock.count == 1);

    lock_release(&test_lock);
    assert(test_lock.owner == NULL);
    assert(test_lock.count == 0);

    kprintf("Passed: Acquiring Single Lock\n");

    kprintf("Test 2: Acquiring Locks Multiple times \n");
    lock_acquire(&test_lock);
    lock_acquire(&test_lock);  // Acquiring twice

    assert(test_lock.owner == TP);
    assert(test_lock.count == 2);
    
    lock_release(&test_lock);
    assert(test_lock.count == 1);
    assert(test_lock.owner == TP);  // Still owned

    lock_release(&test_lock);
    assert(test_lock.owner == NULL);
    assert(test_lock.count == 0);
    kprintf("Passed: Acquiring Locks Multiple times\n");

    kprintf("Test 3: Two Threads trying to lock\n");
    trektid = thread_spawn("thrd1", thread1);
    assert (0 < trektid);

    r30tid = thread_spawn("thrd2", thread2);
    assert (0 < r30tid);
    
    thread_join(0);
    assert(test_lock.owner == NULL);
    assert(test_lock.count == 0); // should be released here after thread2 exits
    assert(test_lock_copy.owner == NULL);
    assert(test_lock_copy.count == 0); // should be released here after thread2 exits
    kprintf("Passed: Two Threads trying to lock\n");
}

void thread1(void) {
    lock_acquire(&test_lock);
    assert(test_lock.owner == TP);
    assert(test_lock.count == 1);
    thread_yield(); // switches to thread2
    lock_release(&test_lock); 
}

void thread2(void) {
    lock_acquire(&test_lock); // condition sleeps until test_lock gets released
    assert(test_lock.owner == TP);
    assert(test_lock.count == 1);
    lock_acquire(&test_lock); 
    assert(test_lock.owner == TP);
    assert(test_lock.count == 2);

    lock_acquire(&test_lock_copy); 
    assert(test_lock_copy.owner == TP);
    assert(test_lock_copy.count == 1); // exits without releasing locks
}
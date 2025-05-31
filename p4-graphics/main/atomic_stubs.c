// main/atomic_stubs.c
#include <stdint.h>

// Definimos __atomic_test_and_set usando __sync_lock_test_and_set de GCC
int __atomic_test_and_set(volatile void *ptr, int memorder) {
    // reinterpretamos ptr como un int y hacemos la operación atómica
    return __sync_lock_test_and_set((volatile int *)ptr, 1);
}

// Definimos __atomic_clear usando __sync_lock_release
void __atomic_clear(volatile void *ptr, int memorder) {
    __sync_lock_release((volatile int *)ptr);
}

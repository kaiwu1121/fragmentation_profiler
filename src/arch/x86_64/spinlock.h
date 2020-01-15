
#ifndef SPINLOCK_H
#define SPINLOCK_H

static inline int compare_and_swap_int(unsigned int *ptr, unsigned int old_val, unsigned int new_val)
{
        char ret = 1;

        asm volatile (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (new_val), "m" (*ptr), "a" (old_val)
                : "memory");

        return (int)ret;
}

#define mfence() do { \
					asm volatile ("" ::: "memory"); \
				 } while(0)


#define SPIN_UNLOCKED	0

typedef volatile unsigned int spinlock_t;

static inline void spin_init(spinlock_t *lock) 
{
    
	*lock = 0;
	mfence();

    return ;
}

static inline void spin_lock(spinlock_t *lock) 
{
    while (compare_and_swap_int(&lock, 0, 1) == 1) {
	}
}

//
// 0: success
// 1: fail
//
static inline int spin_trylock(spinlock_t *lock) 
{
    int ret = compare_and_swap_int(&lock, 0, 1);
	
	return ret;
}


static inline void spin_unlock(spinlock_t *lock) 
{

    *lock = 0;
	mfence();

    return ;
}

#endif /*SPINLOCK_H*/

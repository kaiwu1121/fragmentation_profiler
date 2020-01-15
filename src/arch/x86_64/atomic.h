
#ifndef ATOMIC_H
#define ATOMIC_H

typedef volatile unsigned long atomic_t;

#define ATOMIC_INIT(i)	(i) 

static inline void atomic_inc(atomic_t *val)
{
	asm volatile(
			"lock\n"
			"incl %0"
			: "+m" (val));
}

static inline unsigned long atomic_read(atomic_t *val)
{
	return *val;
}

static inline unsigned long atomic_read_ulong(atomic_t *val)
{

}


#endif /*ATOMIC_H*/


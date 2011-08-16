/* MECHANICALLY GENERATED, DO NOT EDIT!!! */

#ifndef _INCLUDE_API_H
#define _INCLUDE_API_H

/*
 * common.h: Common Linux kernel-isms.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; but version 2 of the License only due
 * to code included from the Linux kernel.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) 2006 Paul E. McKenney, IBM.
 *
 * Much code taken from the Linux kernel.  For such code, the option
 * to redistribute under later versions of GPL might not be available.
 */

#include <urcu/arch.h>

#ifndef __always_inline
#define __always_inline inline
#endif

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)

#ifdef __ASSEMBLY__
#  define stringify_in_c(...)   __VA_ARGS__
#  define ASM_CONST(x)          x
#else
/* This version of stringify will deal with commas... */
#  define __stringify_in_c(...) #__VA_ARGS__
#  define stringify_in_c(...)   __stringify_in_c(__VA_ARGS__) " "
#  define __ASM_CONST(x)        x##UL
#  define ASM_CONST(x)          __ASM_CONST(x)
#endif


/*
 * arch-ppc64.h: Expose PowerPC atomic instructions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; but version 2 of the License only due
 * to code included from the Linux kernel.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) 2006 Paul E. McKenney, IBM.
 *
 * Much code taken from the Linux kernel.  For such code, the option
 * to redistribute under later versions of GPL might not be available.
 */

/*
 * Machine parameters.
 */

#define ____cacheline_internodealigned_in_smp \
	__attribute__((__aligned__(CAA_CACHE_LINE_SIZE)))

/*
 * api_pthreads.h: API mapping to pthreads environment.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.  However, please note that much
 * of the code in this file derives from the Linux kernel, and that such
 * code may not be available except under GPLv2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) 2006 Paul E. McKenney, IBM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#define __USE_GNU
#include <pthread.h>
#include <sched.h>
#include <sys/param.h>
/* #include "atomic.h" */

/*
 * Default machine parameters.
 */

#ifndef CAA_CACHE_LINE_SIZE
/* #define CAA_CACHE_LINE_SIZE 128 */
#endif /* #ifndef CAA_CACHE_LINE_SIZE */

/*
 * Exclusive locking primitives.
 */

typedef pthread_mutex_t spinlock_t;

#define DEFINE_SPINLOCK(lock) spinlock_t lock = PTHREAD_MUTEX_INITIALIZER;
#define __SPIN_LOCK_UNLOCKED(lockp) PTHREAD_MUTEX_INITIALIZER

static void spin_lock_init(spinlock_t *sp)
{
	if (pthread_mutex_init(sp, NULL) != 0) {
		perror("spin_lock_init:pthread_mutex_init");
		exit(-1);
	}
}

static void spin_lock(spinlock_t *sp)
{
	if (pthread_mutex_lock(sp) != 0) {
		perror("spin_lock:pthread_mutex_lock");
		exit(-1);
	}
}

static void spin_unlock(spinlock_t *sp)
{
	if (pthread_mutex_unlock(sp) != 0) {
		perror("spin_unlock:pthread_mutex_unlock");
		exit(-1);
	}
}

#define spin_lock_irqsave(l, f) do { f = 1; spin_lock(l); } while (0)
#define spin_unlock_irqrestore(l, f) do { f = 0; spin_unlock(l); } while (0)

/*
 * Thread creation/destruction primitives.
 */

typedef pthread_t thread_id_t;

#define NR_THREADS 128

#define __THREAD_ID_MAP_EMPTY 0
#define __THREAD_ID_MAP_WAITING 1
thread_id_t __thread_id_map[NR_THREADS];
spinlock_t __thread_id_map_mutex;

#define for_each_thread(t) \
	for (t = 0; t < NR_THREADS; t++)

#define for_each_running_thread(t) \
	for (t = 0; t < NR_THREADS; t++) \
		if ((__thread_id_map[t] != __THREAD_ID_MAP_EMPTY) && \
		    (__thread_id_map[t] != __THREAD_ID_MAP_WAITING))

#define for_each_tid(t, tid) \
	for (t = 0; t < NR_THREADS; t++) \
		if ((((tid) = __thread_id_map[t]) != __THREAD_ID_MAP_EMPTY) && \
		    ((tid) != __THREAD_ID_MAP_WAITING))

pthread_key_t thread_id_key;

static int __smp_thread_id(void)
{
	int i;
	thread_id_t tid = pthread_self();

	for (i = 0; i < NR_THREADS; i++) {
		if (__thread_id_map[i] == tid) {
			long v = i + 1;  /* must be non-NULL. */

			if (pthread_setspecific(thread_id_key, (void *)v) != 0) {
				perror("pthread_setspecific");
				exit(-1);
			}
			return i;
		}
	}
	spin_lock(&__thread_id_map_mutex);
	for (i = 0; i < NR_THREADS; i++) {
		if (__thread_id_map[i] == tid)
			spin_unlock(&__thread_id_map_mutex);
			return i;
	}
	spin_unlock(&__thread_id_map_mutex);
	fprintf(stderr, "smp_thread_id: Rogue thread, id: %d(%#x)\n",
			(int)tid, (int)tid);
	exit(-1);
}

static int smp_thread_id(void)
{
	void *id;

	id = pthread_getspecific(thread_id_key);
	if (id == NULL)
		return __smp_thread_id();
	return (long)(id - 1);
}

static thread_id_t create_thread(void *(*func)(void *), void *arg)
{
	thread_id_t tid;
	int i;

	spin_lock(&__thread_id_map_mutex);
	for (i = 0; i < NR_THREADS; i++) {
		if (__thread_id_map[i] == __THREAD_ID_MAP_EMPTY)
			break;
	}
	if (i >= NR_THREADS) {
		spin_unlock(&__thread_id_map_mutex);
		fprintf(stderr, "Thread limit of %d exceeded!\n", NR_THREADS);
		exit(-1);
	}
	__thread_id_map[i] = __THREAD_ID_MAP_WAITING;
	spin_unlock(&__thread_id_map_mutex);
	if (pthread_create(&tid, NULL, func, arg) != 0) {
		perror("create_thread:pthread_create");
		exit(-1);
	}
	__thread_id_map[i] = tid;
	return tid;
}

static void *wait_thread(thread_id_t tid)
{
	int i;
	void *vp;

	for (i = 0; i < NR_THREADS; i++) {
		if (__thread_id_map[i] == tid)
			break;
	}
	if (i >= NR_THREADS){
		fprintf(stderr, "wait_thread: bad tid = %d(%#x)\n",
				(int)tid, (int)tid);
		exit(-1);
	}
	if (pthread_join(tid, &vp) != 0) {
		perror("wait_thread:pthread_join");
		exit(-1);
	}
	__thread_id_map[i] = __THREAD_ID_MAP_EMPTY;
	return vp;
}

static void wait_all_threads(void)
{
	int i;
	thread_id_t tid;

	for (i = 1; i < NR_THREADS; i++) {
		tid = __thread_id_map[i];
		if (tid != __THREAD_ID_MAP_EMPTY &&
		    tid != __THREAD_ID_MAP_WAITING)
			(void)wait_thread(tid);
	}
}

static void run_on(int cpu)
{
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}

/*
 * timekeeping -- very crude -- should use MONOTONIC...
 */

long long get_microseconds(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		abort();
	return ((long long)tv.tv_sec) * 1000000LL + (long long)tv.tv_usec;
}

/*
 * Per-thread variables.
 */

#define DEFINE_PER_THREAD(type, name) \
	struct { \
		__typeof__(type) v \
			__attribute__((__aligned__(CAA_CACHE_LINE_SIZE))); \
	} __per_thread_##name[NR_THREADS];
#define DECLARE_PER_THREAD(type, name) extern DEFINE_PER_THREAD(type, name)

#define per_thread(name, thread) __per_thread_##name[thread].v
#define __get_thread_var(name) per_thread(name, smp_thread_id())

#define init_per_thread(name, v) \
	do { \
		int __i_p_t_i; \
		for (__i_p_t_i = 0; __i_p_t_i < NR_THREADS; __i_p_t_i++) \
			per_thread(name, __i_p_t_i) = v; \
	} while (0)

/*
 * CPU traversal primitives.
 */

#ifndef NR_CPUS
#define NR_CPUS 16
#endif /* #ifndef NR_CPUS */

#define for_each_possible_cpu(cpu) \
	for (cpu = 0; cpu < NR_CPUS; cpu++)
#define for_each_online_cpu(cpu) \
	for (cpu = 0; cpu < NR_CPUS; cpu++)

/*
 * Per-CPU variables.
 */

#define DEFINE_PER_CPU(type, name) \
	struct { \
		__typeof__(type) v \
			__attribute__((__aligned__(CAA_CACHE_LINE_SIZE))); \
	} __per_cpu_##name[NR_CPUS]
#define DECLARE_PER_CPU(type, name) extern DEFINE_PER_CPU(type, name)

DEFINE_PER_THREAD(int, smp_processor_id);

#define per_cpu(name, thread) __per_cpu_##name[thread].v
#define __get_cpu_var(name) per_cpu(name, smp_processor_id())

#define init_per_cpu(name, v) \
	do { \
		int __i_p_c_i; \
		for (__i_p_c_i = 0; __i_p_c_i < NR_CPUS; __i_p_c_i++) \
			per_cpu(name, __i_p_c_i) = v; \
	} while (0)

/*
 * CPU state checking (crowbarred).
 */

#define idle_cpu(cpu) 0
#define in_softirq() 1
#define hardirq_count() 0
#define PREEMPT_SHIFT   0
#define SOFTIRQ_SHIFT   (PREEMPT_SHIFT + PREEMPT_BITS)
#define HARDIRQ_SHIFT   (SOFTIRQ_SHIFT + SOFTIRQ_BITS)
#define PREEMPT_BITS    8
#define SOFTIRQ_BITS    8

/*
 * CPU hotplug.
 */

struct notifier_block {
	int (*notifier_call)(struct notifier_block *, unsigned long, void *);
	struct notifier_block *next;
	int priority;
};

#define CPU_ONLINE		0x0002 /* CPU (unsigned)v is up */
#define CPU_UP_PREPARE		0x0003 /* CPU (unsigned)v coming up */
#define CPU_UP_CANCELED		0x0004 /* CPU (unsigned)v NOT coming up */
#define CPU_DOWN_PREPARE	0x0005 /* CPU (unsigned)v going down */
#define CPU_DOWN_FAILED		0x0006 /* CPU (unsigned)v NOT going down */
#define CPU_DEAD		0x0007 /* CPU (unsigned)v dead */
#define CPU_DYING		0x0008 /* CPU (unsigned)v not running any task,
				        * not handling interrupts, soon dead */
#define CPU_POST_DEAD		0x0009 /* CPU (unsigned)v dead, cpu_hotplug
					* lock is dropped */

/* Used for CPU hotplug events occuring while tasks are frozen due to a suspend
 * operation in progress
 */
#define CPU_TASKS_FROZEN	0x0010

#define CPU_ONLINE_FROZEN	(CPU_ONLINE | CPU_TASKS_FROZEN)
#define CPU_UP_PREPARE_FROZEN	(CPU_UP_PREPARE | CPU_TASKS_FROZEN)
#define CPU_UP_CANCELED_FROZEN	(CPU_UP_CANCELED | CPU_TASKS_FROZEN)
#define CPU_DOWN_PREPARE_FROZEN	(CPU_DOWN_PREPARE | CPU_TASKS_FROZEN)
#define CPU_DOWN_FAILED_FROZEN	(CPU_DOWN_FAILED | CPU_TASKS_FROZEN)
#define CPU_DEAD_FROZEN		(CPU_DEAD | CPU_TASKS_FROZEN)
#define CPU_DYING_FROZEN	(CPU_DYING | CPU_TASKS_FROZEN)

/* Hibernation and suspend events */
#define PM_HIBERNATION_PREPARE	0x0001 /* Going to hibernate */
#define PM_POST_HIBERNATION	0x0002 /* Hibernation finished */
#define PM_SUSPEND_PREPARE	0x0003 /* Going to suspend the system */
#define PM_POST_SUSPEND		0x0004 /* Suspend finished */
#define PM_RESTORE_PREPARE	0x0005 /* Going to restore a saved image */
#define PM_POST_RESTORE		0x0006 /* Restore failed */

#define NOTIFY_DONE		0x0000		/* Don't care */
#define NOTIFY_OK		0x0001		/* Suits me */
#define NOTIFY_STOP_MASK	0x8000		/* Don't call further */
#define NOTIFY_BAD		(NOTIFY_STOP_MASK|0x0002)
						/* Bad/Veto action */
/*
 * Clean way to return from the notifier and stop further calls.
 */
#define NOTIFY_STOP		(NOTIFY_OK|NOTIFY_STOP_MASK)

/*
 * Bug checks.
 */

#define BUG_ON(c) do { if (!(c)) abort(); } while (0)

/*
 * Initialization -- Must be called before calling any primitives.
 */

static void smp_init(void)
{
	int i;

	spin_lock_init(&__thread_id_map_mutex);
	__thread_id_map[0] = pthread_self();
	for (i = 1; i < NR_THREADS; i++)
		__thread_id_map[i] = __THREAD_ID_MAP_EMPTY;
	init_per_thread(smp_processor_id, 0);
	if (pthread_key_create(&thread_id_key, NULL) != 0) {
		perror("pthread_key_create");
		exit(-1);
	}
}

/* Taken from the Linux kernel source tree, so GPLv2-only!!! */

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

#if 0

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct cds_list_head {
	struct cds_list_head *next, *prev;
};

#define CDS_LIST_HEAD_INIT(name) { &(name), &(name) }

#define CDS_LIST_HEAD(name) \
	struct cds_list_head name = CDS_LIST_HEAD_INIT(name)

static inline void CDS_INIT_LIST_HEAD(struct cds_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#ifndef CONFIG_DEBUG_LIST
static inline void __cds_list_add(struct cds_list_head *new,
			      struct cds_list_head *prev,
			      struct cds_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
#else
extern void __cds_list_add(struct cds_list_head *new,
			      struct cds_list_head *prev,
			      struct cds_list_head *next);
#endif

/**
 * cds_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void cds_list_add(struct cds_list_head *new, struct cds_list_head *head)
{
	__cds_list_add(new, head, head->next);
}


/**
 * cds_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void cds_list_add_tail(struct cds_list_head *new, struct cds_list_head *head)
{
	__cds_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __cds_list_del(struct cds_list_head * prev, struct cds_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * cds_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: cds_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#ifndef CONFIG_DEBUG_LIST
static inline void cds_list_del(struct cds_list_head *entry)
{
	__cds_list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}
#else
extern void cds_list_del(struct cds_list_head *entry);
#endif

/**
 * cds_list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void cds_list_replace(struct cds_list_head *old,
				struct cds_list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void cds_list_replace_init(struct cds_list_head *old,
					struct cds_list_head *new)
{
	cds_list_replace(old, new);
	CDS_INIT_LIST_HEAD(old);
}

/**
 * cds_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void cds_list_del_init(struct cds_list_head *entry)
{
	__cds_list_del(entry->prev, entry->next);
	CDS_INIT_LIST_HEAD(entry);
}

/**
 * cds_list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void cds_list_move(struct cds_list_head *list, struct cds_list_head *head)
{
	__cds_list_del(list->prev, list->next);
	cds_list_add(list, head);
}

/**
 * cds_list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void cds_list_move_tail(struct cds_list_head *list,
				  struct cds_list_head *head)
{
	__cds_list_del(list->prev, list->next);
	cds_list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct cds_list_head *list,
				const struct cds_list_head *head)
{
	return list->next == head;
}

/**
 * cds_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int cds_list_empty(const struct cds_list_head *head)
{
	return head->next == head;
}

/**
 * cds_list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using cds_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is cds_list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int cds_list_empty_careful(const struct cds_list_head *head)
{
	struct cds_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int list_is_singular(const struct cds_list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct cds_list_head *list,
		struct cds_list_head *head, struct cds_list_head *entry)
{
	struct cds_list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct cds_list_head *list,
		struct cds_list_head *head, struct cds_list_head *entry)
{
	if (cds_list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		CDS_INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

static inline void __cds_list_splice(const struct cds_list_head *list,
				 struct cds_list_head *prev,
				 struct cds_list_head *next)
{
	struct cds_list_head *first = list->next;
	struct cds_list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * cds_list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void cds_list_splice(const struct cds_list_head *list,
				struct cds_list_head *head)
{
	if (!cds_list_empty(list))
		__cds_list_splice(list, head, head->next);
}

/**
 * cds_list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void cds_list_splice_tail(struct cds_list_head *list,
				struct cds_list_head *head)
{
	if (!cds_list_empty(list))
		__cds_list_splice(list, head->prev, head);
}

/**
 * cds_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void cds_list_splice_init(struct cds_list_head *list,
				    struct cds_list_head *head)
{
	if (!cds_list_empty(list)) {
		__cds_list_splice(list, head, head->next);
		CDS_INIT_LIST_HEAD(list);
	}
}

/**
 * cds_list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void cds_list_splice_tail_init(struct cds_list_head *list,
					 struct cds_list_head *head)
{
	if (!cds_list_empty(list)) {
		__cds_list_splice(list, head->prev, head);
		CDS_INIT_LIST_HEAD(list);
	}
}

/**
 * cds_list_entry - get the struct for this entry
 * @ptr:	the &struct cds_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define cds_list_entry(ptr, type, member) \
	caa_container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	cds_list_entry((ptr)->next, type, member)

/**
 * cds_list_for_each	-	iterate over a list
 * @pos:	the &struct cds_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define cds_list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); \
        	pos = pos->next)

/**
 * __cds_list_for_each	-	iterate over a list
 * @pos:	the &struct cds_list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant differs from cds_list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __cds_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * cds_list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct cds_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define cds_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
        	pos = pos->prev)

/**
 * cds_list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct cds_list_head to use as a loop cursor.
 * @n:		another &struct cds_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define cds_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * cds_list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct cds_list_head to use as a loop cursor.
 * @n:		another &struct cds_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define cds_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/**
 * cds_list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define cds_list_for_each_entry(pos, head, member)				\
	for (pos = cds_list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = cds_list_entry(pos->member.next, typeof(*pos), member))

/**
 * cds_list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define cds_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = cds_list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head); 	\
	     pos = cds_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in cds_list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in cds_list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : cds_list_entry(head, typeof(*pos), member))

/**
 * cds_list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define cds_list_for_each_entry_continue(pos, head, member) 		\
	for (pos = cds_list_entry(pos->member.next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = cds_list_entry(pos->member.next, typeof(*pos), member))

/**
 * cds_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define cds_list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = cds_list_entry(pos->member.prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = cds_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * cds_list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define cds_list_for_each_entry_from(pos, head, member) 			\
	for (; prefetch(pos->member.next), &pos->member != (head);	\
	     pos = cds_list_entry(pos->member.next, typeof(*pos), member))

/**
 * cds_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define cds_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = cds_list_entry((head)->next, typeof(*pos), member),	\
		n = cds_list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = cds_list_entry(n->member.next, typeof(*n), member))

/**
 * cds_list_for_each_entry_safe_continue
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define cds_list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = cds_list_entry(pos->member.next, typeof(*pos), member), 		\
		n = cds_list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = cds_list_entry(n->member.next, typeof(*n), member))

/**
 * cds_list_for_each_entry_safe_from
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define cds_list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = cds_list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = cds_list_entry(n->member.next, typeof(*n), member))

/**
 * cds_list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define cds_list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = cds_list_entry((head)->prev, typeof(*pos), member),	\
		n = cds_list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = cds_list_entry(n->member.prev, typeof(*n), member))

#endif //0

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct cds_hlist_head {
	struct cds_hlist_node *first;
};

struct cds_hlist_node {
	struct cds_hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct cds_hlist_head name = {  .first = NULL }
#define CDS_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct cds_hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const struct cds_hlist_node *h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct cds_hlist_head *h)
{
	return !h->first;
}

static inline void __cds_hlist_del(struct cds_hlist_node *n)
{
	struct cds_hlist_node *next = n->next;
	struct cds_hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void cds_hlist_del(struct cds_hlist_node *n)
{
	__cds_hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void cds_hlist_del_init(struct cds_hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__cds_hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void cds_hlist_add_head(struct cds_hlist_node *n, struct cds_hlist_head *h)
{
	struct cds_hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct cds_hlist_node *n,
					struct cds_hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void hlist_add_after(struct cds_hlist_node *n,
					struct cds_hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

/*
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move_list(struct cds_hlist_head *old,
				   struct cds_hlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define cds_hlist_entry(ptr, type, member) caa_container_of(ptr,type,member)

#define cds_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos && ({ prefetch(pos->next); 1; }); \
	     pos = pos->next)

#define cds_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

/**
 * cds_hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct cds_hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the cds_hlist_node within the struct.
 */
#define cds_hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = cds_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * cds_hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct cds_hlist_node to use as a loop cursor.
 * @member:	the name of the cds_hlist_node within the struct.
 */
#define cds_hlist_for_each_entry_continue(tpos, pos, member)		 \
	for (pos = (pos)->next;						 \
	     pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = cds_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * cds_hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct cds_hlist_node to use as a loop cursor.
 * @member:	the name of the cds_hlist_node within the struct.
 */
#define cds_hlist_for_each_entry_from(tpos, pos, member)			 \
	for (; pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = cds_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * cds_hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct cds_hlist_node to use as a loop cursor.
 * @n:		another &struct cds_hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the cds_hlist_node within the struct.
 */
#define cds_hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) && 				 \
		({ tpos = cds_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)

#endif

#endif

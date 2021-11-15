#ifndef ARR_H
#define ARR_H

#include <stdbool.h> /* bool, true, false */
#include <stdlib.h> /* malloc(), realloc(), free() */
#include <string.h> /* memmove() */

/* Simple array */
#define ARR(T) struct { size_t len; const size_t sz; T *arr; }

/* Fixed array (pre-defined size) */
#define FARR(T, elnum) struct { size_t len; const size_t sz; T arr[(elnum)]; }

/* Dynamic array */
#define DARR(T) struct { size_t len, sz; T *arr; } *

/* The *_NEW macros return newly-constructed (allocated + initialized) arrays.
 * Their parameters are of the type of the array and the number of elements it must have.
 */

#define ARR_NEW(T, elnum) { \
	.len = 0, \
	.sz = (elnum) * sizeof(T), \
	.arr = (T [(elnum)]) {0} \
}

#define FARR_NEW(T, elnum) { \
	.len = 0, \
	.sz = (elnum) * sizeof(T), \
	.arr = {0} \
}

static inline void *darr_new(size_t elnum, size_t elsz)
{
	if(elnum && elsz && ((size_t)-1) / elsz >= elnum) {
		DARR(void) new = malloc(sizeof(*new));
		if(new) {
			new->arr = malloc(elnum * elsz);
			if(new->arr) {
				new->len = 0;
				new->sz = elnum * elsz;
				return new;
			}
			free(new);
		}
	}
	return NULL;
}

/* May return NULL upon allocation failure */
#define DARR_NEW(T, elnum) darr_new((elnum), sizeof(T))

/* free()'s allocations made for a dynamic array */
static void darr_del(void *foo)
{
	if(foo) {
		DARR(void) tofree = foo;
		free(tofree->arr);
		free(tofree);
	}
}

/* Convenience boolean macro to test if array is full */
#define ARR_FULL(T, name) ((name).len * sizeof(T) == (name).sz)
#define FARR_FULL ARR_FULL
#define DARR_FULL(T, name) ARR_FULL(T, *name)

/* Convenience macro that returns number of elements that may still be stored in array */
#define ARR_AVAILIBLE(T, name) ( ((name).sz - (name).len * sizeof(T)) / sizeof(T) )
#define FARR_AVAILIBLE ARR_AVAILIBLE
#define DARR_AVAILIBLE(T, name) ARR_AVAILIBLE(T, *name)

/* Append an object of type T to array "name".
 * Noop if array is full.
 */
#define ARR_PUSH(T, name, ...) do { \
	if(!ARR_FULL(T, name)) \
		(name).arr[(name).len++] = (T){__VA_ARGS__}; \
} while(0)
#define FARR_PUSH ARR_PUSH

#define DARR_SZMAX ((size_t)-1)

static inline size_t darr_max(size_t x, size_t y)
{
	return x > y? x : y;
}

/* Realloc DARR foo->arr by a factor of 1.5 */ 
static inline bool darr_grow(void *foo)
{
	if(foo) {
		DARR(void) old = foo;
		/* ((size_t)-1) is SIZE_MAX */
		/* Overflow check */
		size_t max = darr_max(old->sz/2, 1);
		if(DARR_SZMAX - old->sz >= max) {
			void *new_arr = realloc(old->arr, old->sz + max);
			if(new_arr) {
				old->arr = new_arr;
				old->sz += max;
				return true; 
			}
		}
	}
	return false;
}

/* Append *elem to DARR foo->arr, growing it if necessary */
static inline bool darr_push(void *foo, size_t elsz, const void *elem)
{
	if(foo) {
		DARR(void) bar = foo;
		size_t effsz = bar->len * elsz;
		/* Overflow check */
		if(DARR_SZMAX - effsz >= elsz) { 
			while(effsz + elsz > bar->sz) {
				if(!darr_grow(bar))
					return false;
			}
			memmove((char *)bar->arr + effsz, elem, elsz);
			bar->len++;
			return true;
		}
	}
	return false;
}

/* Append an object of type T to dyanmic array, growing it if necessary */
#define DARR_PUSH(T, name, ...) darr_push(name, sizeof(T), &(T){__VA_ARGS__})

#define ARR_POP(name) do { \
	if((name).len) \
		(name).len--; \
} while(0)
#define FARR_POP ARR_POP
#define DARR_POP(name) ARR_POP(*name)

#define ARR_CLEAR(name) do { \
	(name).len = 0; \
} while(0)
#define FARR_CLEAR ARR_CLEAR
#define DARR_CLEAR(name) ARR_CLEAR(*name)

#endif

#ifndef _NEWBOS_ORDERED_ARRAY_H
#define _NEWBOS_ORDERED_ARRAY_H

#include <stdint.h>

/*
 * This array is insertion sorted - it always remains in a sorted state
 * (between calls). It can store anything that can be cast to void * -- so a
 * uint32_t, or any pointer
 */
typedef void *type_t;

/*
 * A predicate should return non-zero if the fierst argument is less than the
 * second. Else it should return zero.
 */
typedef int8_t (*less_than_predicate_t)(type_t, type_t);

typedef struct
{
    type_t *array;
    uint32_t size;
    uint32_t max_size;
    less_than_predicate_t less_than;
} ordered_array_t;

/*
 * A standard less than predicate.
 */
int8_t
standard_less_than_predicate(
    type_t a,
    type_t b
);

/*
 * Create an ordered array.
 */
ordered_array_t
create_ordereed_array(
    uint32_t max_size,
    less_than_predicate_t less_than
);

ordered_array_t
place_ordered_array(
    void *address,
    uint32_t max_size,
    less_than_predicate_t less_than
);

/*
 * Destroy an ordered array.
 */
void
destroy_ordered_array(
    ordered_array_t *array
);

/*
 * Add an item into the array.
 */
void
insert_ordered_array(
    type_t item,
    ordered_array_t *array
);

/*
 * Lookup the item at index i.
 */
type_t
lookup_ordered_array(
    uint32_t index,
    ordered_array_t *array
);

/*
 * Delete the item at index i.
 */
void
remove_ordered_array(
    uint32_t index,
    ordered_array_t *array
);

#endif

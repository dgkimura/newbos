#include <newbos/ordered_array.h>

int8_t
standard_less_than_predicate(
    type_t a,
    type_t b)
{
    return (a < b) ? 1 : 0;
}

ordered_array_t
place_ordered_array(
    void *address,
    uint32_t max_size,
    less_than_predicate_t less_than)
{
    ordered_array_t to_ret;
    to_ret.array = (type_t *)address;
    to_ret.size = 0;
    to_ret.max_size = max_size;
    to_ret.less_than = less_than;
    return to_ret;
}

void
insert_ordered_array(
    type_t item,
    ordered_array_t *array)
{
    uint32_t index = 0;
    for (; index < array->size &&
           array->less_than(array->array[index], item); index++);

    if (index == array->size)
    {
        /*
         * Add item to the end of the array
         */
        array->array[array->size++] = item;
    }
    else
    {
        type_t tmp = array->array[index];
        array->array[index] = item;

        while (index < array->size)
        {
            index += 1;
            type_t tmp2 = array->array[index];
            array->array[index] = tmp;
            tmp = tmp2;
        }
        array->size += 1;
    }
}

type_t
lookup_ordered_array(
    uint32_t index,
    ordered_array_t *array)
{
    return array->array[index];
}

void
remove_ordered_array(
    uint32_t index,
    ordered_array_t *array)
{
    while (index < array->size)
    {
        array->array[index] = array->array[index+1];
        index += 1;
    }
    array->size -= 1;
}

#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray *array)
{
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray *array, Value value)
{
  if (array->capacity < array->count + 1)
  {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values =
        GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void freeValueArray(ValueArray *array)
{
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value)
{
  switch (value.type)
  {
  case VAL_BOOL:
    printf("<bool|%s>", AS_BOOL(value) ? "true" : "false");
    break;
  case VAL_NIL:
    printf("<nil|nil>");
    break;
  case VAL_BYTE:
    printf("<byte|%d>", AS_BYTE(value));
    break;
  case VAL_INT:
    printf("<int|%d>", AS_INT(value));
    break;
  case VAL_FLOAT:
    printf("<float|%f>", AS_FLOAT(value));
    break;
  case VAL_OBJ:
    printObject(value);
    break;
  }
}

bool valuesEqual(Value a, Value b)
{
  if (a.type != b.type)
    return false;

  switch (a.type)
  {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NIL:
    return true;
  case VAL_BYTE:
    return AS_BYTE(a) == AS_BYTE(b);
  case VAL_INT:
    return AS_INT(a) == AS_INT(b);
  case VAL_FLOAT:
    return AS_FLOAT(a) == AS_FLOAT(b);
  case VAL_OBJ:
    return AS_OBJ(a) == AS_OBJ(b);
  default:
    return false; // Unreachable.
  }
}
#ifndef xasm_value_h
#define xasm_value_h

#include "common.h"

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_BYTE,
  VAL_INT,
  VAL_FLOAT,
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    char byte;
    int i;
    float f;
  } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_BYTE(value) ((value).type == VAL_BYTE)
#define IS_INT(value) ((value).type == VAL_INT)
#define IS_FLOAT(value) ((value).type == VAL_FLOAT)
#define IS_NUMBER(value)                                                       \
  ((value).type == VAL_BYTE || (value).type == VAL_INT ||                      \
   (value).type == VAL_FLOAT)

#define IS_TYPE_NUMBER(value)                                                  \
  (value == VAL_BYTE || value == VAL_INT || value == VAL_FLOAT)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_BYTE(value) ((value).as.byte)
#define AS_INT(value) ((value).as.i)
#define AS_FLOAT(value) ((value).as.f)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.byte = 0}})
#define BYTE_VAL(value) ((Value){VAL_BYTE, {.byte = value}})
#define INT_VAL(value) ((Value){VAL_INT, {.i = value}})
#define FLOAT_VAL(value) ((Value){VAL_FLOAT, {.f = value}})

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif
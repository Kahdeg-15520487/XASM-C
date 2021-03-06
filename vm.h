#ifndef xasm_vm_h
#define xasm_vm_h

#include "chunk.h"
#include "table.h"

#define STACK_MAX 256

typedef struct
{
  Chunk *chunk;
  uint8_t *ip;
  Value stack[STACK_MAX];
  int stackCount;
  Value *stackTop;
  Table strings;
  Table globals;
  bool OverflowFlag;

  Obj *objects;
} VM;

typedef enum
{
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
void freeObjects();
InterpretResult interpretChunk(Chunk *chunk);
InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif
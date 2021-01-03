#ifndef xasm_vm_h
#define xasm_vm_h

#include "chunk.h"

#define STACK_MAX 256

typedef struct {
  Chunk *chunk;
  uint8_t *ip;
  Value stack[STACK_MAX];
  int stackCount;
  Value* stackTop;
  bool OverflowFlag;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpretChunk(Chunk *chunk);
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
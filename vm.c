#include <stdio.h>

#include "common.h"
#include "value.h"
#include "vm.h"
#include "compiler.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.stackCount = 0;
  vm.OverflowFlag = false;
}

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) {
  if (vm.stackCount + 1 == STACK_MAX) {
    vm.OverflowFlag = true;
    return;
  }
  *vm.stackTop = value;
  vm.stackTop++;
  vm.stackCount++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    char b = pop();                                                            \
    char a = pop();                                                            \
    push(a op b);                                                              \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_RET: {
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }

    case OP_YEET: {
      Value constant = READ_CONSTANT();
      push(constant);
      if (vm.OverflowFlag) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }

    case OP_NEG: {
      push(-pop());
      break;
    }
    case OP_ADD: {
      BINARY_OP(+);
      break;
    }
    case OP_SUB: {
      BINARY_OP(-);
      break;
    }
    case OP_MUL: {
      BINARY_OP(*);
      break;
    }
    case OP_DIV: {
      BINARY_OP(/);
      break;
    }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpretChunk(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}

InterpretResult interpret(const char* source) {
  compile(source);
  return INTERPRET_OK;

}


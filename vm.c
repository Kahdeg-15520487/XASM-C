#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.stackCount = 0;
  vm.OverflowFlag = false;
}

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  resetStack();
}

void initVM() {
  resetStack();
  vm.objects = NULL;

  initTable(&vm.globals);
  initTable(&vm.strings);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

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
  vm.stackCount--;
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  ObjString *b = AS_STRING(pop());
  ObjString *a = AS_STRING(pop());

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString *result = takeString(chars, length);
  push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(op, isComparison)                                            \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    Value b = pop();                                                           \
    Value a = pop();                                                           \
    printf("\n");                                                              \
    switch (a.type) {                                                          \
    case VAL_INT: {                                                            \
      int va = AS_INT(a);                                                      \
      int vb = 0;                                                              \
      switch (b.type) {                                                        \
      case VAL_BYTE: {                                                         \
        vb = AS_BYTE(b);                                                       \
        break;                                                                 \
      }                                                                        \
      case VAL_INT: {                                                          \
        vb = AS_INT(b);                                                        \
        break;                                                                 \
      }                                                                        \
      case VAL_FLOAT: {                                                        \
        vb = (int)AS_FLOAT(b);                                                 \
        break;                                                                 \
      }                                                                        \
      default:                                                                 \
        runtimeError("Operand must be a number.");                             \
        break;                                                                 \
      }                                                                        \
      if (isComparison)                                                        \
        push(BOOL_VAL(va op vb));                                              \
      else                                                                     \
        push(INT_VAL(va op vb));                                               \
      break;                                                                   \
    }                                                                          \
    case VAL_BYTE: {                                                           \
      char va = AS_BYTE(a);                                                    \
      char vb = 0;                                                             \
      switch (b.type) {                                                        \
      case VAL_BYTE: {                                                         \
        vb = AS_BYTE(b);                                                       \
        break;                                                                 \
      }                                                                        \
      case VAL_INT: {                                                          \
        vb = (char)AS_INT(b);                                                  \
        break;                                                                 \
      }                                                                        \
      case VAL_FLOAT: {                                                        \
        vb = (char)AS_FLOAT(b);                                                \
        break;                                                                 \
      }                                                                        \
      default:                                                                 \
        runtimeError("Operand must be a number.");                             \
        break;                                                                 \
      }                                                                        \
      if (isComparison)                                                        \
        push(BOOL_VAL(va op vb));                                              \
      else                                                                     \
        push(BYTE_VAL(va op vb));                                              \
      break;                                                                   \
    }                                                                          \
    case VAL_FLOAT: {                                                          \
      float va = AS_FLOAT(a);                                                  \
      float vb = 0;                                                            \
      switch (b.type) {                                                        \
      case VAL_BYTE: {                                                         \
        vb = (float)AS_BYTE(b);                                                \
        break;                                                                 \
      }                                                                        \
      case VAL_INT: {                                                          \
        vb = (float)AS_INT(b);                                                 \
        break;                                                                 \
      }                                                                        \
      case VAL_FLOAT: {                                                        \
        vb = (float)AS_FLOAT(b);                                               \
        break;                                                                 \
      }                                                                        \
      default:                                                                 \
        runtimeError("Operand must be a number.");                             \
        break;                                                                 \
      }                                                                        \
      if (isComparison)                                                        \
        push(BOOL_VAL(va op vb));                                              \
      else                                                                     \
        push(FLOAT_VAL(va op vb));                                             \
      break;                                                                   \
    }                                                                          \
    default:                                                                   \
      runtimeError("Operand must be a number.");                               \
      break;                                                                   \
    }                                                                          \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("          ");
      printf("[ ");
      printValue(*slot);
      printf(" ]\n");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_RET: {
      return INTERPRET_OK;
    }

    // flow control
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      vm.ip += offset;
      break;
    }
    case OP_JUMP_IF_TRUE: {
      uint16_t offset = READ_SHORT();
      if (!isFalsey(peek(0)))
        vm.ip += offset;
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(peek(0)))
        vm.ip += offset;
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      vm.ip -= offset;
      break;
    }

    // scope management
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      tableSet(&vm.globals, name, peek(0));
      pop();
      break;
    }
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(vm.stack[slot]);
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      vm.stack[slot] = peek(0);
      break;
    }

      // stack manipulation
    case OP_YEET: {
      Value constant = READ_CONSTANT();
      push(constant);
      if (vm.OverflowFlag) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_POP: {
      pop();
      break;
    }

    case OP_NEG: {
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      Value inp = pop();
      switch (inp.type) {
      case VAL_BYTE:
        push(BYTE_VAL(-AS_BYTE(inp)));
        break;
      case VAL_INT:
        push(INT_VAL(-AS_INT(inp)));
        break;
      case VAL_FLOAT:
        push(FLOAT_VAL(-AS_FLOAT(inp)));
        break;

      default:
        break;
      }
      break;
    }
    case OP_ADD: {
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        BINARY_OP(+, false);
      } else {
        runtimeError("Operands must be two numbers or two strings.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SUB: {
      BINARY_OP(-, false);
      break;
    }
    case OP_MUL: {
      BINARY_OP(*, false);
      break;
    }
    case OP_DIV: {
      BINARY_OP(/, false);
      break;
    }

      // literal
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;

    // boolean
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
      break;

    // comparison:
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER:
      BINARY_OP(>, true);
      break;

    case OP_LESS:
      BINARY_OP(<, true);
      break;

    // system
    case OP_PRINT: {
      printValue(pop());
      printf("\n");
      break;
    }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpretChunk(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}

InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}

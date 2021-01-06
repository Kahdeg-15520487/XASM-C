#ifndef xasm_chunk_h
#define xasm_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_JMP,
  OP_JE,
  OP_JNE,
  OP_JG,
  OP_JGE,
  OP_JL,
  OP_JLE,
  OP_J1,
  OP_J0,
  OP_JEL,
  OP_JNEL,

  OP_EXIT,
  OP_PAUSE,
  OP_HALT,
  OP_UNHALT,

  OP_NOP,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_DEC,
  OP_INC,
  OP_NEG,

  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_NOT,

  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_DEFINE_GLOBAL,

  OP_PUSH,
  OP_YEET,
  OP_POP,
  OP_LOAD,
  OP_GETSTACK,
  OP_SETSTACK,
  OP_COPY,
  OP_DUP,
  OP_SWAP,
  OP_REMOVE,

  OP_RAND,
  OP_RANDSEED,
  OP_RANDMAX,
  OP_RANDRANGE,

  OP_CALL,
  OP_RET,
  OP_BRP,
  OP_REQ,
  OP_HOST,
  OP_PRINT,
} OpCode;

typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  int *lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);
void freeChunk(Chunk *chunk);

#endif
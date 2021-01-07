#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
  printf("== %s ==\n", name);
}

static int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int byteInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

static int jumpInstruction(const char *name, int sign, Chunk *chunk,
                           int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}

int disassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_JUMP:
    return jumpInstruction("OP_jump", 1, chunk, offset);
  case OP_JUMP_IF_FALSE:
    return jumpInstruction("OP_jumpfalse", 1, chunk, offset);
  case OP_JUMP_IF_TRUE:
    return jumpInstruction("OP_jumptrue", 1, chunk, offset);
  case OP_LOOP:
    return jumpInstruction("OP_LOOP", -1, chunk, offset);

  case OP_EXIT:
    return simpleInstruction("OP_exit", offset);
  case OP_PAUSE:
    return simpleInstruction("OP_pause", offset);
  case OP_HALT:
    return simpleInstruction("OP_halt", offset);
  case OP_UNHALT:
    return simpleInstruction("OP_unhalt", offset);

  case OP_NOP:
    return simpleInstruction("OP_nop", offset);

  case OP_ADD:
    return simpleInstruction("OP_add", offset);
  case OP_SUB:
    return simpleInstruction("OP_sub", offset);
  case OP_MUL:
    return simpleInstruction("OP_mul", offset);
  case OP_DIV:
    return simpleInstruction("OP_div", offset);
  case OP_MOD:
    return simpleInstruction("OP_mod", offset);
  case OP_DEC:
    return simpleInstruction("OP_dec", offset);
  case OP_INC:
    return simpleInstruction("OP_inc", offset);
  case OP_NEG:
    return simpleInstruction("OP_neg", offset);
  case OP_EQUAL:
    return simpleInstruction("OP_eq", offset);
  case OP_GREATER:
    return simpleInstruction("OP_gt", offset);
  case OP_LESS:
    return simpleInstruction("OP_lt", offset);
  case OP_NOT:
    return simpleInstruction("OP_NOT", offset);

  case OP_NIL:
    return simpleInstruction("OP_NIL", offset);
  case OP_TRUE:
    return simpleInstruction("OP_TRUE", offset);
  case OP_FALSE:
    return simpleInstruction("OP_FALSE", offset);

  case OP_DEFINE_GLOBAL:
    return constantInstruction("OP_def_global", chunk, offset);
  case OP_GET_GLOBAL:
    return constantInstruction("OP_get_global", chunk, offset);
  case OP_SET_GLOBAL:
    return constantInstruction("OP_set_global", chunk, offset);
  case OP_GET_LOCAL:
    return byteInstruction("OP_get_local", chunk, offset);
  case OP_SET_LOCAL:
    return byteInstruction("OP_set_local", chunk, offset);

  case OP_PUSH:
    return simpleInstruction("OP_push", offset);
  case OP_YEET:
    return constantInstruction("OP_yeet", chunk, offset);
  case OP_POP:
    return simpleInstruction("OP_pop", offset);
  case OP_LOAD:
    return simpleInstruction("OP_load", offset);
  case OP_GETSTACK:
    return simpleInstruction("OP_getstack", offset);
  case OP_SETSTACK:
    return simpleInstruction("OP_setstack", offset);
  case OP_COPY:
    return simpleInstruction("OP_copy", offset);
  case OP_DUP:
    return simpleInstruction("OP_dup", offset);
  case OP_SWAP:
    return simpleInstruction("OP_swap", offset);
  case OP_REMOVE:
    return simpleInstruction("OP_remove", offset);

  case OP_RAND:
    return simpleInstruction("OP_rand", offset);
  case OP_RANDSEED:
    return simpleInstruction("OP_randseed", offset);
  case OP_RANDMAX:
    return simpleInstruction("OP_randmax", offset);
  case OP_RANDRANGE:
    return simpleInstruction("OP_randrange", offset);

  case OP_CALL:
    return simpleInstruction("OP_call", offset);
  case OP_RET:
    return simpleInstruction("OP_ret", offset);
  case OP_BRP:
    return simpleInstruction("OP_brp", offset);
  case OP_REQ:
    return simpleInstruction("OP_req", offset);
  case OP_HOST:
    return simpleInstruction("OP_host", offset);
  case OP_PRINT:
    return simpleInstruction("OP_PRINT", offset);

  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
#ifndef xasm_compiler_h
#define xasm_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);

#endif
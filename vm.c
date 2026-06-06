#include <stdio.h>
#define DS_IMPLEMENTATION
#include "ds.h"

#define PAL_IMPLEMENTATION
#include "pal.h"

#undef DS_IMPLEMENTATION
#undef PAL_IMPLEMENTATION

typedef struct decoder decoder;
struct decoder {
    u8 *at;
    u8 *end;
};

typedef enum opcode {
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,

    OPCODE_MOV,
    OPCODE_JUMP,
}opcode;

typedef struct instruction instruction;
struct instruction {
    opcode opc;
    i32 left_operand;
    i32 right_operand;
};

instruction get_instruction(decoder *decoder) {
    instruction inst = {0};

    return inst;
}

int main(int argc, char **argv) {

    u64 file_size;
    unsigned char *instructions = pal_read_entire_file("main.bc", &file_size);

    decoder dcoder = {0};
    dcoder.at = instructions;
    dcoder.end = instructions + file_size;


    return 0;
}

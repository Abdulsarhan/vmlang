#ifndef WRITER_H
#define WRITER_H

#include "ds.h"

#define TWOCC(x)((uint16_t)(x[1] << 8 | x[0]))
#define FOURCC(x)((uint32_t)(x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0]))

typedef struct exe_writer exe_writer;
struct exe_writer {
    u8 *buffer;
    u64 at;
    u64 buffer_size;
    u8 bit_pos;
};

exe_writer writer_init(mem_arena *arena, u64 buffer_size);
void write_bits_aligned(exe_writer *writer, u8 *source, u64 bits_to_write);
void write_bits(exe_writer *writer, u8 *source, u64 bits_to_write);
void write_bytes(exe_writer *writer, const u8 *source, u64 size);
void write_u8(exe_writer *writer, u8 value);
void write_u16(exe_writer *writer, u16 value);
void write_u32(exe_writer *writer, u32 value);
void write_u64(exe_writer *writer, u64 value);

#endif // WRITER_H

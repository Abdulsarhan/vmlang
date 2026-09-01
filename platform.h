#ifndef PLATFORM_H
#define PLATFORM_H
#include "ds.h"

string8 read_entire_file(mem_arena *arena, string8 file_path);
b32 write_entire_file(mem_arena *arena, string8 file_path, u8 *buffer, u64 buffer_size);

#endif //PLATFORM_H

#include "elf_file.h"
#include "writer.h"

void generate_elf_file(ast_node *root,
                  string8 output_path,
                  const u8 *text,
                  u32 size_of_text,
                  const u8 *data,
                  u32 size_of_data,
                  const u8 *rodata,
                  u32 size_of_rodata,
                  u32 size_of_bss) {
    (void)root;

    mem_arena *arena = arena_init(gibibytes(1));
}

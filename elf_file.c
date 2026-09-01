#include "elf_file.h"
#include "writer.h"
#include "platform.h"

#define osabi_sysv 0x0

#define elf_type_exec 0x02
#define elf_type_dyn 0x03
#define machine_x86 0x03
#define machine_x64 0x3E

#pragma pack(push, 1)
typedef struct elf_header_32 elf_header_32;
struct elf_header_32 {
    u32 e_magic;
    u8  e_class;
    u8  e_data;
    u8  e_version;
    u8  e_osabi;
    u8  e_abiver;
    u8  e_pad[7];
    u16 e_type;
    u16 e_machine;
    u32 e_version2;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

typedef struct elf_header_64 elf_header_64;
struct elf_header_64 {
    u32 e_magic;
    u8  e_class;
    u8  e_data;
    u8  e_version;
    u8  e_osabi;
    u8  e_abiver;
    u8  e_pad[7];
    u16 e_type;
    u16 e_machine;
    u32 e_version2;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};
#pragma pack(pop)

typedef enum elf_type {
    elf_type_32,
    elf_type_64,
} elf_type;

typedef struct program_header_entry_32 program_header_entry_32;
struct program_header_entry_32 {
    u32 p_type;
    u32 p_offset;
    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filesz;
    u32 p_memsz;
    u32 p_flags;
    u32 p_align;
};
typedef struct program_header_entry_64 program_header_entry_64;
struct program_header_entry_64 {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
};

typedef struct section_header_entry_32 section_header_entry_32;
struct section_header_entry_32 {
    u32 sh_name;
    u32 sh_type;
    u32 sh_flags;
    u32 sh_addr;
    u32 sh_offset;
    u32 sh_size;
    u32 sh_link;
    u32 sh_info;
    u32 sh_addralign;
    u32 sh_entsize;
};

typedef struct section_header_entry_64 section_header_entry_64;
struct section_header_entry_64 {
    u32 sh_name;
    u32 sh_type;
    u64 sh_flags;
    u64 sh_addr;
    u64 sh_offset;
    u64 sh_size;
    u32 sh_link;
    u32 sh_info;
    u64 sh_addralign;
    u64 sh_entsize;
};

void write_elf_header_32(exe_writer *writer, u32 absolute_addr_of_entry_point,
                         u32 start_of_section_headers, u16 num_sections,
                         u16 num_program_entries, u16 section_name_index) {
    elf_header_32 header;
    header.e_magic = 0x464c457f; // 0x7f 'E' 'L' 'F' (backwards cuz little endian)
    header.e_class = 1;
    header.e_data = 1;
    header.e_version = 1;
    header.e_osabi = osabi_sysv;
    header.e_abiver = 0;
    memory_set(&header.e_pad, 0, sizeof(header.e_pad));
    header.e_type = elf_type_exec;
    header.e_machine = machine_x86;
    header.e_version2 = 1;
    header.e_entry = absolute_addr_of_entry_point;
    header.e_phoff = 0x34;
    header.e_shoff = start_of_section_headers;
    header.e_flags = 0;
    header.e_ehsize = sizeof(elf_header_32);
    header.e_phentsize = sizeof(program_header_entry_32);
    header.e_phnum = num_program_entries;
    header.e_shentsize =  sizeof(section_header_entry_32);
    header.e_shnum = num_sections;
    header.e_shstrndx = section_name_index;
    write_bytes(writer, (const u8*)&header, sizeof(elf_header_32));
}

void write_elf_header_64(exe_writer *writer, u64 absolute_addr_of_entry_point,
                         u64 start_of_section_headers, u16 num_sections,
                         u16 num_program_entries, u16 section_name_index) {
    elf_header_64 header;
    header.e_magic = 0x464c457f; // 0x7f 'E' 'L' 'F' (backwards cuz little endian)
    header.e_class = 2;
    header.e_data = 1;
    header.e_version = 1;
    header.e_osabi = osabi_sysv;
    header.e_abiver = 0;
    memory_set(&header.e_pad, 0, sizeof(header.e_pad));
    header.e_type = elf_type_exec;
    header.e_machine = machine_x64;
    header.e_version = 1;
    header.e_entry = absolute_addr_of_entry_point;
    header.e_phoff = 0x40;
    header.e_shoff = start_of_section_headers;
    header.e_flags = 0;
    header.e_ehsize = sizeof(elf_header_64);
    header.e_phentsize = sizeof(program_header_entry_64);
    header.e_phnum = num_program_entries;
    header.e_shentsize =  sizeof(section_header_entry_64);
    header.e_shnum = num_sections;
    header.e_shstrndx = section_name_index;
    write_bytes(writer, (const u8*)&header, sizeof(elf_header_64));
}

void generate_elf_file(ast_node *root, string8 output_path, generated_code *code) {
    (void)root;

    elf_type type = elf_type_64;
    mem_arena *arena = arena_init(gibibytes(1));

    exe_writer elf_writer = writer_init(arena, mebibytes(10));
    // NOTE: We are just getting rid of this while we figure out PE stuff.
#if 0
    if (type == elf_type_64) {
        write_elf_header_64(&elf_writer);
    } else if(type == elf_type_32) {
        write_elf_header_32(&elf_writer);
    }
#endif

    write_entire_file(arena, output_path, elf_writer.buffer, elf_writer.at);
}

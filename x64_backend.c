#include <windows.h>

#include "x64_backend.h"

#include "pal.h"

#define TWOCC(x)((uint16_t)(x[1] << 8 | x[0]))
#define FOURCC(x)((uint32_t)(x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0]))

#define IMAGE_BASE_32 0x000400000
#define IMAGE_BASE_64 0x140000000
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2 // windows gui subsystem
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3 // windows console subsystem

#define IMAGE_FILE_EXECUTABLE_IMAGE 0x002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200

#define OPTIONAL_HEADER_PE32 0x10B
#define OPTIONAL_HEADER_PE32_PLUS 0x20B


#define text_flags IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ
#define data_flags IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define bss_flags IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define idata_flags IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define rodata_flags IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ
#define rdata_flags rodata_flags

typedef struct {
    u32 signature;
    u16 machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_sym_table;
    u16 size_of_optional_header;
    u16 characteristsics;
}coff_header;

typedef struct {
    u16 magic;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 addr_of_entry_point;
    u32 base_of_code;
}standard_coff_fields_64;

typedef struct {
    u8    section_name[8];
    u32   virtual_size;
    u32   virtual_address;
    u32   size_of_raw_data;
    u32   pointer_to_raw_data;
    u32   pointer_to_relocations;
    u32   pointer_to_numbers;
    u16   number_of_relocations;
    u16   number_of_line_numbers;
    u32   characteristics;
}image_section_header;

typedef struct import_dir_entry import_dir_entry;
struct import_dir_entry {
    u32 import_lookup_rva;
    u32 time_date_stamp;
    u32 forwarder_chain;
    u32 name_rva;
    u32 thunk_table_rva;
};

typedef struct pe_data_directory pe_data_directory;
struct pe_data_directory {
    u32 virtual_address;
    u32 size;
};

typedef enum pe_object_kind {
    pe_object_kind_obj,
    pe_object_kind_exe,
}pe_object_kind;

typedef enum pe_format {
    pe_format_pe32,
    pe_format_pe32_plus,
}pe_format;

typedef enum encoding_form {
    enc_mr,
    enc_rm,
    enc_mi,
    enc_oi,
    enc_i,
    enc_m,
    enc_zo
} encoding_form;

typedef struct instruction_form instruction_form;
struct instruction_form {
    uint8_t  opcode[3];
    uint8_t  opcode_len;
    uint8_t  opcode_ext;   // ModRM.reg extension for /digit forms, e.g. SUB = 5
    encoding_form  form;
    uint8_t  imm_size;     // 0, 1, 2, 4, or 8
    b32     needs_rexw;
};

static const instruction_form sub_rm64_imm8 = {
    .opcode = {0x83}, .opcode_len = 1, .opcode_ext = 5,
    .form = enc_mi, .imm_size = 1, .needs_rexw = true,
};

typedef struct exe_writer exe_writer;
struct exe_writer {
    u8 *buffer;
    u64 at;
    u64 buffer_size;
    u8 bit_pos;
};

void write_bits_aligned(exe_writer *writer, u8 *source, u64 bits_to_write) {
    u64 whole_bytes = bits_to_write / 8;
    u8 trailing_bits = (u8)(bits_to_write % 8);

    u64 space = writer->buffer_size - writer->at;
    u64 to_copy = whole_bytes < space ? whole_bytes : space;
    memcpy(&writer->buffer[writer->at], source, to_copy);
    writer->at += to_copy;

    if (to_copy < whole_bytes) {
        return; // ran out of space partway through whole bytes
    }

    if (trailing_bits && writer->at < writer->buffer_size) {
        u8 mask = (u8)((1u << trailing_bits) - 1);
        writer->buffer[writer->at] = source[whole_bytes] & mask;
        writer->bit_pos = trailing_bits;
    }
}

void write_bits(exe_writer *writer, u8 *source, u64 bits_to_write) {
    // If we're currently byte-aligned, the fast path handles
    // everything — whole bytes via memcpy plus any trailing partial
    // byte. There's nothing left for the bit loop to do afterward.
    if (writer->bit_pos == 0) {
        write_bits_aligned(writer, source, bits_to_write);
        return;
    }

    // Bit-by-bit path, only reachable when bit_pos != 0 on entry.
    for (u64 i = 0; i < bits_to_write; i++) {
        if (writer->at >= writer->buffer_size) {
            return; // out of space
        }

        u64 byte_index = i / 8;
        u8 bit_index = (u8)(i % 8);
        u8 bit = (source[byte_index] >> bit_index) & 1;

        if (writer->bit_pos == 0) {
            writer->buffer[writer->at] = 0;
        }

        writer->buffer[writer->at] |= (u8)(bit << writer->bit_pos);

        writer->bit_pos++;
        if (writer->bit_pos == 8) {
            writer->bit_pos = 0;
            writer->at++;
        }
    }
}

void write_bytes(exe_writer *writer, const u8 *source, u64 size) {
    if(writer->at + size < writer->buffer_size) {
        memcpy(&writer->buffer[writer->at], source, size);
        writer->at += size;
    } else {
        printf("not going to write");
    }
}

void write_u8(exe_writer *writer, u8 value) {
    if(writer->at + sizeof(u8) < writer->buffer_size) {
        writer->buffer[writer->at++] = value;
    } else {
        printf("not going to write");
    }
}

void write_u16(exe_writer *writer, u16 value) {
    if(writer->at + sizeof(u16) < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += sizeof(u16);
    } else {
        printf("not going to write");
    }
}

void write_u32(exe_writer *writer, u32 value) {
    if(writer->at + sizeof(u32) < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += sizeof(u32);
    } else {
        printf("not going to write");
    }
}

void write_u64(exe_writer *writer, u64 value) {
    if(writer->at + sizeof(u64) < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += sizeof(u64);
    } else {
        printf("not going to write");
    }
}

void write_data_directory(exe_writer *writer, pe_data_directory directory) {
    if(writer->at + 8 < writer->buffer_size) {
        memcpy(&writer->buffer[writer->at], &directory, sizeof(pe_data_directory));
        writer->at += 8;
    } else {
        printf("not going to write");
    }
}

void write_dos_header(exe_writer *writer) {
    write_u16(writer, TWOCC("MZ")); // write dos header signature
    write_u16(writer, 0x80);
    write_u16(writer, 0x1);
    write_u16(writer, 0x0);
    write_u16(writer, 0x4);
    write_u16(writer, 0x10);
    write_u16(writer, 0xFFFF);
    write_u16(writer, 0x0);
    write_u16(writer, 0x140);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x40);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u16(writer, 0x0);
    write_u32(writer, 0x80);
    assert(writer->at == 0x40);
}

void write_dos_stub(exe_writer *writer) {
    const u8 stub_start[] = {0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21};
    const string8 stub_middle = STR8_LIT("This program cannot be run in DOS mode.");
    const u8 stub_end[] = {0x2E, 0x0D, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    write_bytes(writer, stub_start, array_count(stub_start));
    write_bytes(writer, stub_middle.data, stub_middle.length);
    write_bytes(writer, stub_end, array_count(stub_end));
}

u64 get_unix_time() {
   const u64 unix_time_start = 0x019DB1DED53E8000; //January 1, 1970 (start of Unix epoch) in "ticks"
   const u64 ticks_per_second = 10000000; //a tick is 100ns

   FILETIME ft;
   GetSystemTimeAsFileTime(&ft); //returns ticks in UTC

   LARGE_INTEGER time;
   time.LowPart  = ft.dwLowDateTime;
   time.HighPart = ft.dwHighDateTime;

   return (time.QuadPart - unix_time_start) / ticks_per_second;
}

void write_coff_header(exe_writer *writer, pe_format format, pe_object_kind kind, b32 has_debug_info, u16 num_sections) {
    write_u32(writer, FOURCC("PE\0\0")); // write PE SIG
    write_u16(writer, (u16)IMAGE_FILE_MACHINE_AMD64);
    write_u16(writer, num_sections);
    u32 unix_timestamp = get_unix_time();
    write_u32(writer, unix_timestamp);
    write_u32(writer, 0); // pointer to sym table (deprecated)
    write_u32(writer, 0); // num symbols  (deprecated)
    u16 size_of_optional_header = 0;
    if(kind == pe_object_kind_exe) {
        if(format == pe_format_pe32_plus) {
            size_of_optional_header = 240; assert(0); // TODO: This is wrong, it doesn't take into account the section headers.
        } else if(format == pe_format_pe32) {
            size_of_optional_header = 224; assert(0); // TODO: This is wrong, it doesn't take into account the section headers.
        }
    } else if(kind == pe_object_kind_obj) {
        size_of_optional_header = 0;
    }
    write_u16(writer, size_of_optional_header);

    u16 characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE | has_debug_info ? 0 : IMAGE_FILE_DEBUG_STRIPPED;
    write_u16(writer, characteristics);
}

void write_standard_coff_fields(exe_writer *writer,
                                pe_format kind,
                                u32 size_of_code,
                                u32 size_of_initialized_data,
                                u32 size_of_uninitialized_data,
                                u32 address_of_entry_point,
                                u32 base_of_code,
                                u32 base_of_data) {
    write_u16(writer, (u16)OPTIONAL_HEADER_PE32_PLUS);
    // NOTE: these version numbers are wrong, but windows doesn't seem to care.
    write_u8(writer, 0); // major linker version
    write_u8(writer, 0); // minor linker version
    write_u32(writer, size_of_code);
    write_u32(writer, size_of_initialized_data);
    write_u32(writer, size_of_uninitialized_data);
    write_u32(writer, address_of_entry_point);
    write_u32(writer, base_of_code);
    if(kind == pe_format_pe32) {
        write_u32(writer, base_of_data);
    }
    if(kind == pe_format_pe32) {
        assert(writer->at == 0x80 + 28);
    } else if(kind == pe_format_pe32_plus) {
        assert(writer->at == 0x80 + 24);
    }
}

u16 pe_checksum(u32 ulPartialSum, u16 *start_of_pe_file, u32 file_length) {
    do {
        if ((file_length)) {
            ulPartialSum += *start_of_pe_file++;
        }
        ulPartialSum = (ulPartialSum >> 16) + (ulPartialSum & 0xffff);
    } while (file_length--);
    return (u16)ulPartialSum;
}

void write_windows_specific_fields(exe_writer *writer,
                                   u32 section_align,
                                   u32 file_align,
                                   u32 size_of_headers,
                                   u32 size_of_image,
                                   u16 subsystem,
                                   b32 has_debug_info) {
    write_u64(writer, IMAGE_BASE_64);
    write_u32(writer, section_align);
    write_u32(writer, file_align);
    assert(section_align >= file_align && "section alignment has to be greater than or equal to file alignment");

    // we need to do some manifest bullshit in order
    // to get version 10 (current version)
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
    u16 windows_version_major = 6;
    u16 windows_version_minor = 2;
    // os version
    write_u16(writer, windows_version_major);
    write_u16(writer, windows_version_minor);

    // this .exe's version
    write_u16(writer, 0);
    write_u16(writer, 0);

    // subsystem version
    write_u16(writer, windows_version_major);
    write_u16(writer, windows_version_minor);

    write_u32(writer, 0); // reserved field

    write_u32(writer, size_of_image);

    write_u32(writer, size_of_headers);

    u32 checksum = 0; assert(0); // TODO: Maybe we can ignore this.
                                 // documentation says that only dlls and drivers get their checksum validated.
    write_u32(writer, checksum);

    write_u16(writer, subsystem);
    write_u16(writer, 0); // dll_characteristics
    write_u64(writer, mebibytes(1)); // stack reserve
    write_u64(writer, kibibytes(4)); // stack commit
    write_u64(writer, mebibytes(1)); // heap reserve
    write_u64(writer, kibibytes(1)); // heap commit
    write_u32(writer, 0);
    u32 num_rva_and_sizes = 0; assert(0); // TODO: Wrong, fix.
    write_u32(writer, num_rva_and_sizes);

    // at this point, this is what the offset should be for a 64 bit executable
    assert(writer->at == 0x80 + 112);

    write_data_directory(writer, (pe_data_directory){0, 0}); // export
    write_data_directory(writer, (pe_data_directory){0, 0}); // import
    write_data_directory(writer, (pe_data_directory){0, 0}); // resources
    write_data_directory(writer, (pe_data_directory){0, 0}); // exception
    write_data_directory(writer, (pe_data_directory){0, 0}); // Certificates
    write_data_directory(writer, (pe_data_directory){0, 0}); // base relocs
    pe_data_directory debug_dir;
    debug_dir.size = 0;
    debug_dir.virtual_address = 0;
    if(has_debug_info) {
        //TODO: put the correct values in here for debug builds.
        debug_dir.size = 0;
        debug_dir.virtual_address = 0;
    }
    write_data_directory(writer, debug_dir); // debug info
    write_data_directory(writer, (pe_data_directory){0, 0}); // Architecture (reserved, must be zero)
    write_data_directory(writer, (pe_data_directory){0, 0}); // Global Pointer Register (not a real thing on x64)
    write_data_directory(writer, (pe_data_directory){0, 0}); // Thread Local Storage
    write_data_directory(writer, (pe_data_directory){0, 0}); // Load Config Table
    write_data_directory(writer, (pe_data_directory){0, 0}); // Bound Import (for dlls, I think)
    write_data_directory(writer, (pe_data_directory){0, 0}); // IAT
    write_data_directory(writer, (pe_data_directory){0, 0}); // Delay Import Desc (not used, I think)
    write_data_directory(writer, (pe_data_directory){0, 0}); // CLR runtime header and size. I assume that this should be zero for us since we are not using the CLR.
    write_data_directory(writer, (pe_data_directory){0, 0}); // must be zero

    // at this point, this is what the offset should be for a 64 bit executable
    assert(writer->at == 0x80 + 240);
}

void write_image_section_header(exe_writer *writer, const char *section_name, pe_object_kind kind, u32 file_align, u32 virtual_size, u32 virtual_address, u32 size_of_raw_data, u16 num_relocations, u32 characteristics) {
    char name[8];
    memset(name, 0, 8);
    strcpy(name, section_name);
    write_u64(writer, *(u64*)&name);

    if(kind == pe_object_kind_obj) {
        virtual_size = 0;
    }

    write_u32(writer, virtual_size);
    write_u32(writer, virtual_address);

    assert(size_of_raw_data % file_align == 0);
    write_u32(writer, size_of_raw_data);

    u32 ptr_to_relocs = 0;
    if(kind == pe_object_kind_exe) {
        ptr_to_relocs = 0;
    }
    write_u32(writer, ptr_to_relocs);

    write_u32(writer, 0); // has to be zero

    if(kind == pe_object_kind_exe) {
        num_relocations = 0;
    }

    write_u16(writer, num_relocations);
    write_u16(writer, 0); // Has to be zero
    write_u32(writer, characteristics);
}

exe_writer writer_init(mem_arena *arena, u64 buffer_size) {
    exe_writer writer;
    writer.at = 0;
    writer.buffer_size = buffer_size;
    writer.buffer = ARENA_PUSH_ARRAY(arena, u8, writer.buffer_size);
    return writer;
}

void write_import_dir_entry(exe_writer *writer, u32 import_lookup_rva, u32 time_date_stamp, u32 forwarder_chain, u32 name_rva, u32 thunk_table_rva) {
    write_u32(writer, import_lookup_rva);
    write_u32(writer, time_date_stamp);
    write_u32(writer, forwarder_chain);
    write_u32(writer, name_rva);
    write_u32(writer, thunk_table_rva);
}

void write_hint_name_table_entry(exe_writer *writer, string8 function_name) {
    u32 checkpoint = writer->at;
    write_u16(writer, 0);
    write_bytes(writer, function_name.data, function_name.length);
    write_u8(writer, 0); // null terminator for string.
    u32 size = writer->at - checkpoint;
    if(size % 2 != 0) {
        // padding byte to make the whole thing align on an even boundary.
        write_u8(writer, 0);
    }
}

void write_import_lookup_table_entry(exe_writer *writer, pe_format format, u32 hint_name_table_rva) {

    write_bits(writer, (u8*)&hint_name_table_rva, 31);

    u32 zeroes = 0;
    if(format == pe_format_pe32_plus) {
        write_bits(writer, (u8*)&zeroes, 32);
    }

    u8 ordinal_flag = 0;
    write_bits(writer, &ordinal_flag, 1);
}

void write_import_functions(string8 *function_names, u32 function_count, exe_writer *idata_section_writer, pe_format format, u32 idata_section_rva) {
    u32 offset = idata_section_rva + idata_section_writer->at;
    // we are calculating the offset of where the function names that we want to import
    // are stored in the idata section. We do this by taking our current writer's position,
    // and then adding to it the number of IAT and INT entries (writer->function_count * 2),
    // multiplied by the size of each entry (8 on pe32+ and 4 on pe32).
    if(format == pe_format_pe32_plus) {
        offset += 8 * function_count * 2;
    } else if(format == pe_format_pe32) {
        offset += 4 * function_count * 2;
    }

    u32 starting_offset = offset;

    for(int i = 0; i < function_count; i++) {
        write_import_lookup_table_entry(idata_section_writer, pe_format_pe32_plus, offset);
        offset += function_names[i].length;
    }
    write_import_lookup_table_entry(idata_section_writer, pe_format_pe32_plus, 0); // null entry

    offset = starting_offset;
    for(int i = 0; i < function_count; i++) {
        write_import_lookup_table_entry(idata_section_writer, pe_format_pe32_plus, offset);
        offset += function_names[i].length;
    }
    write_import_lookup_table_entry(idata_section_writer, pe_format_pe32_plus, 0); // null entry

    for (int i = 0; i < function_count; i++) {
        write_hint_name_table_entry(idata_section_writer, function_names[i]);
    }

    function_count = 0;
}

typedef struct section_writer section_writer;
struct section_writer {

};

void generate_pe_file(ast_node *root,
                      string8 output_path,
                      string8 *import_function_names,
                      u32 import_function_count) {
    (void)root;

    const pe_format format = pe_format_pe32_plus;
    const pe_object_kind kind = pe_object_kind_exe;
    const u32 section_align = 4096;
    const u32 file_align = 512;
    const u32 num_sections = 4;
    const b32 is_gui = true;
    const b32 has_debug_info = false;

    mem_arena *arena = arena_init(gibibytes(1));

    // text
    exe_writer text_section_writer = writer_init(arena, mebibytes(10));
    const u8 code[] = {0x48, 0x83, 0xEC, 0x28,                                  // sub rsp, 0x28
                       0xB9, 0xF5, 0xFF, 0xFF, 0xFF,                            // mov ecx, -11
                       0xFF, 0x15, 0x59, 0x30, 0x00, 0x00,                      // call [GetStdHandle]
                       0x48, 0x89, 0xC1,                                        // mov rcx, rax
                       0x48, 0x8D, 0x15, 0xE7, 0x0F, 0x00, 0x00,                // lea rdx, [start of rodata]
                       0x41, 0xB8, 0x0E, 0x00, 0x00, 0x00,                      // mov r8d, 0x0E
                       0x4C, 0x8D, 0x0D, 0xDA, 0x1F, 0x00, 0x00,                // lea r9, [start of data]
                       0x48, 0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00,    // mov qword ptr [rsp + 0x20], 0
                       0xFF, 0x15, 0x2B, 0x30, 0x00, 0x00,                      // call [WriteConsoleA]
                       0x31, 0xC9,                                              // xor ecx, ecx
                       0xFF, 0x15, 0x1B, 0x30, 0x00, 0x00};                     // call [ExitProcess]
    write_bytes(&text_section_writer, code, array_count(code));

    u64 image_base = 0x140000000;
    u32 text_rva = 0x1000;
    u64 start_of_text = image_base + text_rva;

    // rodata (finished)
    u32 rodata_rva = 0x2000;
    u64 start_of_rodata = image_base + rodata_rva;
    exe_writer rodata_section_writer = writer_init(arena, mebibytes(10));
    const char *rodata_section = "Hello from pe!";
    write_bytes(&rodata_section_writer, (u8*)rodata_section, sizeof(rodata_section) - 1);

    // data (finished)
    u32 data_rva = 0x3000;
    u64 start_of_data = image_base + data_rva;
    exe_writer data_section_writer = writer_init(arena, mebibytes(10));
    write_u32(&data_section_writer, 0);

    // idata
    u32 idata_rva = 0x4000;
    u64 start_of_idata = image_base + idata_rva;
    exe_writer idata_section_writer = writer_init(arena, mebibytes(10));
    write_import_dir_entry(&idata_section_writer, idata_rva + 0x38, 0, 0, idata_rva + 0x28, idata_rva + 0x58);
    write_import_dir_entry(&idata_section_writer, 0, 0, 0, 0, 0);           // null descriptor

    assert(idata_section_writer.at == 0x28);

    const u8 kernel_32[] = "Kernel32.dll";
    write_bytes(&idata_section_writer, kernel_32, sizeof(kernel_32));
    u32 offset = idata_rva + idata_section_writer.at;
    u32 num_padding_bytes = (8 - (offset & 7)) & 7;
    for(u32 i = 0; i < num_padding_bytes; i++) {
        write_u8(&idata_section_writer, 0);
    }

    write_import_functions(import_function_names, import_function_count, &idata_section_writer, format, idata_rva);

    exe_writer pe_writer = writer_init(arena, mebibytes(10));

    write_dos_header(&pe_writer);
    write_dos_stub(&pe_writer);

    write_coff_header(&pe_writer, format, kind, has_debug_info, num_sections);

    u32 size_of_text = text_section_writer.at - (u64)text_section_writer.buffer;
    // NOTE: for some reason, in the assembly program that we have, this is 0 even though we have read only data. does read only data not count as initialized data?
    u32 size_of_data = data_section_writer.at - (u64)data_section_writer.buffer;
    u32 size_of_rodata = rodata_section_writer.at - (u64)rodata_section_writer.buffer;
    u32 size_of_idata = idata_section_writer.at - (u64)idata_section_writer.buffer;
    u32 size_of_init_data = size_of_data + size_of_rodata;
    u32 size_of_uninit_data = 0;
    u32 addr_of_entry_rva = text_rva;
    u32 base_of_code_rva = text_rva;
    u32 base_of_data_rva = idata_rva;
    write_standard_coff_fields(&pe_writer, format, size_of_text, size_of_init_data, size_of_uninit_data, addr_of_entry_rva, base_of_code_rva, base_of_data_rva);

    u32 size_of_headers;
    if(format == pe_format_pe32_plus) {
        size_of_headers = 240;
    } else if(format == pe_format_pe32) {
        size_of_headers = 224;
    }
    size_of_headers += num_sections * 40; // each section header is 40 bytes.
    // size of image after it's been mapped into memory.
    u32 size_of_image = size_of_headers + 0x4000;
    write_windows_specific_fields(&pe_writer,
                                  section_align,
                                  file_align,
                                  size_of_headers,
                                  size_of_image,
                                  is_gui ? IMAGE_SUBSYSTEM_WINDOWS_GUI : IMAGE_SUBSYSTEM_WINDOWS_CUI,
                                  has_debug_info);

    write_image_section_header(&pe_writer, ".text", kind, file_align, 0x1000, 0x1000, size_of_text, 0, text_flags);
    write_image_section_header(&pe_writer, ".rodata", kind, file_align, 0x1000, 0x2000, size_of_rodata, 0, rodata_flags);
    write_image_section_header(&pe_writer, ".data", kind, file_align, 0x1000, 0x3000, size_of_data, 0, data_flags);
    write_image_section_header(&pe_writer, ".idata", kind, file_align, 0x1000, 0x4000, size_of_idata, 0, idata_flags);

    pal_write_file(pe_writer.buffer, pe_writer.at - (u64)pe_writer.buffer, str_to_cstr(arena, output_path));
}

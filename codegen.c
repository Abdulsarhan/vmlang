#include "codegen.h"
#include <windows.h>

#define TWOCC(x)((uint16_t)(x[1] << 8 | x[0]))
#define FOURCC(x)((uint32_t)(x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0]))

#define IMAGE_BASE_32 0x000400000
#define IMAGE_BASE_64 0x140000000
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define IMAGE_SUBSYSTEM_NATIVE 1

#define IMAGE_FILE_EXECUTABLE_IMAGE 0x002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200

#define OPTIONAL_HEADER_PE32 0x10B
#define OPTIONAL_HEADER_PE32_PLUS 0x20B

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

typedef struct exe_writer exe_writer;
struct exe_writer {
    u8 *buffer;
    u64 at;
    u64 buffer_size;
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

void write_u8(exe_writer *writer, u8 value) {
    if(writer->at + 1 < writer->buffer_size) {
        writer->buffer[writer->at++] = value;
    } else {
        printf("not going to write");
    }
}

void write_u16(exe_writer *writer, u16 value) {
    if(writer->at + 2 < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += 2;
    } else {
        printf("not going to write");
    }
}

void write_u32(exe_writer *writer, u32 value) {
    if(writer->at + 4 < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += 4;
    } else {
        printf("not going to write");
    }
}

void write_u64(exe_writer *writer, u64 value) {
    if(writer->at + 8 < writer->buffer_size) {
        writer->buffer[writer->at] = value;
        writer->at += 8;
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
    writer->at += 60 - sizeof(u16); // skip dos header (it's legacy)
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

void write_coff_header(exe_writer *writer, pe_object_kind kind, b32 has_debug_info, u16 num_sections) {
    write_u32(writer, FOURCC("PE\0\0")); // write PE SIG
    write_u16(writer, (u16)IMAGE_FILE_MACHINE_AMD64);
    write_u16(writer, num_sections);
    assert(0);
    u32 unix_timestamp = get_unix_time();
    write_u32(writer, unix_timestamp);
    write_u32(writer, 0); // pointer to sym table (deprecated)
    write_u32(writer, 0); // num symbols  (deprecated)
    u16 size_of_optional_header;
    if(kind == pe_object_kind_exe) {
        assert(0);
        // TODO: This is wrong. Put the correct size of the optional header in here.
        size_of_optional_header = 0;
    } else if(kind == pe_object_kind_obj) {
        size_of_optional_header = 0;
    }
    write_u16(writer, size_of_optional_header);

    u16 characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE | has_debug_info ? 0 : IMAGE_FILE_DEBUG_STRIPPED;
    write_u16(writer, characteristics);
}


void write_standard_coff_fields(exe_writer *writer) {
    write_u16(writer, (u16)OPTIONAL_HEADER_PE32_PLUS);
    // NOTE: these versions numbers are wrong, but windows doesn't seem to care.
    write_u8(writer, 0); // major linker version
    write_u8(writer, 0); // minor linker version
    u32 size_of_code = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, size_of_code);
    u32 size_of_initialized_data = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, size_of_initialized_data);
    u32 size_of_uninitialized_data = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, size_of_uninitialized_data);
    u32 addr_of_entry_point = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, addr_of_entry_point);
    u32 base_of_code = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, base_of_code);
    u32 base_of_data = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, base_of_data);
}

void write_windows_specific_fields(exe_writer *writer, u32 section_align, u32 file_align, u16 subsystem, b32 has_debug_info) {
    write_u64(writer, IMAGE_BASE_64);
    write_u32(writer, section_align);
    write_u32(writer, file_align);
    assert(section_align >= file_align && "section alignment has to be greater than or equal to file alignment");

    // we need to do some manifest bullshit in order
    // to get version 10 (current version)
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
    write_u16(writer, 6);
    write_u16(writer, 2);

    write_u16(writer, 0);
    write_u16(writer, 1);

    // TODO: I am assuming that subsystem version is the same
    // thing as the windows version
    write_u16(writer, 6);
    write_u16(writer, 2);

    write_u32(writer, 0); // reserved field

    u32 size_of_image = 0; assert(0); // TODO: This is wrong, fix.
    write_u32(writer, size_of_image);

    u32 size_of_headers = 0; assert(0); // TODO: This is wrong, fix.
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

    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); //NOTE: Certificate table. I assume we don't need this.
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    pe_data_directory debug_dir;
    debug_dir.size = 0;
    debug_dir.virtual_address = 0;
    if(has_debug_info) {
        //TODO: put the correct values in here for debug builds.
        debug_dir.size = 0;
        debug_dir.virtual_address = 0;
    }
    write_data_directory(writer, debug_dir);
    write_data_directory(writer, (pe_data_directory){0, 0}); // reserved, has to be zero
    write_data_directory(writer, (pe_data_directory){0, 0}); // size part of structure must be zero
    write_data_directory(writer, (pe_data_directory){0, 0}); // TLS, optional
    write_data_directory(writer, (pe_data_directory){0, 0}); // load config table, not sure if this is nececssary
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); assert(0); // FIX;
    write_data_directory(writer, (pe_data_directory){0, 0}); // CLR runtime header and size. I assume that this should be zero for us.
    write_data_directory(writer, (pe_data_directory){0, 0}); // must be zero

}

void write_image_section_header(exe_writer *writer, const char *section_name, pe_object_kind kind, u32 file_align) {
    char name[8];
    memset(name, 0, 8);
    strcpy(name, section_name);
    write_u64(writer, *(u64*)&name);

    u32 virtual_size = 0;
    if(kind == pe_object_kind_exe) {
        virtual_size = 0; assert(0); // wrong, fix
    } else if(kind == pe_object_kind_obj) {
        virtual_size = 0;
    }
    write_u32(writer, virtual_size);
    write_u32(writer, 0); assert(0); // wrong, fix
    u32 size_of_raw_data = 0;
    assert(size_of_raw_data % file_align == 0);
    write_u32(writer, size_of_raw_data); assert(0); // wrong, fix
    u32 ptr_to_relocs = 0;
    if(kind == pe_object_kind_exe) {
        ptr_to_relocs = 0;
    } else if(kind == pe_object_kind_obj) {
        ptr_to_relocs = 0; assert(0); // wrong, fix
    }
    write_u32(writer, ptr_to_relocs);
    write_u32(writer, 0); // has to be zero
    u16 num_relocations = 0;
    if(kind == pe_object_kind_exe) {
        num_relocations = 0;
    } else if(kind == pe_object_kind_obj) {
        num_relocations = 0; assert(0); // wrong, fix
    }
    write_u16(writer, num_relocations);
    write_u16(writer, 0); // Has to be zero
    u32 characteristics = 0;
    write_u32(writer, characteristics); assert(0);
}

void generate_code(ast_node *root) {
    (void)root;

    mem_arena *arena = arena_init(gibibytes(1));

    //writer init
    exe_writer writer;
    writer.at = 0;
    writer.buffer_size = mebibytes(2);
    writer.buffer = ARENA_PUSH_ARRAY(arena, u8, writer.buffer_size);

    write_dos_header(&writer);

    pe_object_kind kind = pe_object_kind_exe;
    b32 has_debug_info = false;
    u16 num_sections = 3;
    write_coff_header(&writer, kind, has_debug_info, num_sections);

    write_standard_coff_fields(&writer);

    u32 section_align = 4096;
    u32 file_align = 512;
    b32 is_gui = true;
    write_windows_specific_fields(&writer, section_align, file_align, is_gui ? IMAGE_SUBSYSTEM_WINDOWS_GUI : IMAGE_SUBSYSTEM_NATIVE, has_debug_info);

    write_image_section_header(&writer, ".text", kind, file_align);
    write_image_section_header(&writer, ".data", kind, file_align);
    write_image_section_header(&writer, ".rodata", kind, file_align);

    const char *string_data = "simple PE File!";
    u8 *data = ARENA_PUSH_ARRAY(arena, u8, sizeof(string_data) - 1);
}

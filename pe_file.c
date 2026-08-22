//TODO: Use pal.h for unix time. Don't rely on platform specific shit.
#include <windows.h>

#include "pe_file.h"
#include "writer.h"
#include "ds.h"
#include "pal.h"

#define IMAGE_BASE_32 0x000400000
#define IMAGE_BASE_64 0x140000000
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2 // windows gui subsystem
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3 // windows console subsystem

#undef IMAGE_FILE_EXECUTABLE_IMAGE

#define IMAGE_FILE_EXECUTABLE_IMAGE 0x002
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200

#define OPTIONAL_HEADER_PE32 0x10B
#define OPTIONAL_HEADER_PE32_PLUS 0x20B

#define text_flags   IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ
#define data_flags   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define bss_flags    IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define idata_flags  IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE
#define rodata_flags IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ
#define rdata_flags rodata_flags

#pragma pack(push, 1)
typedef struct dos_header dos_header;
struct dos_header {      // MS-DOS EXE header
    u16   e_magic;       // Magic number: 0x5A4D or MZ
    u16   e_cblp;        // Bytes on last page of file
    u16   e_cp;          // Pages in file
    u16   e_crlc;        // Relocations
    u16   e_cparhdr;     // Size of header, in paragraphs
    u16   e_minalloc;    // Min - extra paragraphs needed
    u16   e_maxalloc;    // Max - extra paragraphs needed
    u16   e_ss;          // Initial (relative) SS value
    u16   e_sp;          // Initial SP value
    u16   e_csum;        // Checksum
    u16   e_ip;          // Initial IP value
    u16   e_cs;          // Initial (relative) CS value
    u16   e_lfarlc;      // File address of relocation table
    u16   e_ovno;        // Overlay number
    u16   e_res[4];      // Reserved u16s
    u16   e_oemid;       // OEM identifier
    u16   e_oeminfo;     // OEM information
    u16   e_res2[10];    // Reserved u16s
    u32   e_lfanew;      // Offset to NT header
};

typedef struct coff_header coff_header;
struct coff_header {
    u32 signature;
    u16 machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_sym_table;
    u16 size_of_optional_header;
    u16 characteristsics;
};

typedef struct standard_coff_fields_32 standard_coff_fields_32;
struct standard_coff_fields_32 {
    u16 magic;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 addr_of_entry_point;
    u32 base_of_code;
    u32 base_of_data;
};

typedef struct standard_coff_fields_64 standard_coff_fields_64;
struct standard_coff_fields_64 {
    u16 magic;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 addr_of_entry_point;
    u32 base_of_code;
};

typedef struct image_data_directory_entry image_data_directory_entry;
struct image_data_directory_entry {
    u32 virtual_address;
    u32 size;
};

typedef struct windows_specific_fields_32 windows_specific_fields_32;
struct windows_specific_fields_32 {
  u32 ImageBase;
  u32 SectionAlignment;
  u32 FileAlignment;
  u16 MajorOperatingSystemVersion;
  u16 MinorOperatingSystemVersion;
  u16 MajorImageVersion;
  u16 MinorImageVersion;
  u16 MajorSubsystemVersion;
  u16 MinorSubsystemVersion;
  u32 Win32VersionValue;
  u32 SizeOfImage;
  u32 SizeOfHeaders;
  u32 CheckSum;
  u16 Subsystem;
  u16 DllCharacteristics;
  u32 SizeOfStackReserve;
  u32 SizeOfStackCommit;
  u32 SizeOfHeapReserve;
  u32 SizeOfHeapCommit;
  u32 LoaderFlags;
  u32 NumberOfRvaAndSizes;
};

typedef struct windows_specific_fields_64 windows_specific_fields_64;
struct windows_specific_fields_64 {
  u64 ImageBase;
  u32 SectionAlignment;
  u32 FileAlignment;
  u16 MajorOperatingSystemVersion;
  u16 MinorOperatingSystemVersion;
  u16 MajorImageVersion;
  u16 MinorImageVersion;
  u16 MajorSubsystemVersion;
  u16 MinorSubsystemVersion;
  u32 Win32VersionValue;
  u32 SizeOfImage;
  u32 SizeOfHeaders;
  u32 CheckSum;
  u16 Subsystem;
  u16 DllCharacteristics;
  u64 SizeOfStackReserve;
  u64 SizeOfStackCommit;
  u64 SizeOfHeapReserve;
  u64 SizeOfHeapCommit;
  u32 LoaderFlags;
  u32 NumberOfRvaAndSizes;
};

typedef struct image_section_header image_section_header;
struct image_section_header {
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
};

typedef struct import_dir_entry import_dir_entry;
struct import_dir_entry {
    u32 import_lookup_rva;
    u32 time_date_stamp;
    u32 forwarder_chain;
    u32 name_rva;
    u32 thunk_table_rva;
};

#pragma pack(pop)

typedef enum pe_object_kind {
    pe_object_kind_obj,
    pe_object_kind_exe,
}pe_object_kind;

typedef enum pe_format {
    pe_format_pe32,
    pe_format_pe32_plus,
}pe_format;

typedef struct section_info section_info;
struct section_info {
    string8 section_name;
    const u8 *section_data;
    u32 rva;
    u32 virtual_size;

    u32 offset_on_disk;
    u32 size_on_disk;
    u32 size_of_raw_data;
};

typedef struct section_list section_list;
struct section_list {
    section_info *infos;
    u32 num_sections;

    u32 import_dir_rva;
    u32 import_dir_size;

    u32 iat_rva;
    u32 iat_size;
};

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

void write_data_directory_entry(exe_writer *writer, image_data_directory_entry directory) {
    if(writer->at + 8 <= writer->buffer_size) {
        memory_copy(&writer->buffer[writer->at], &directory, sizeof(image_data_directory_entry));
        writer->at += 8;
    } else {
        printf("not going to write");
    }
}

void write_dos_header(exe_writer *writer) {
    dos_header header;
    // NOTE: Most of the fields are typically set to 0, so we just do a memset
    // upfront instead of manually setting each field.
    memory_set(&header, 0, sizeof(header));
    header.e_magic = TWOCC("MZ");
    header.e_cblp = 0x80;
    header.e_cp = 0x1;
    header.e_cparhdr = 0x4;
    header.e_minalloc = 0x10;
    header.e_maxalloc = 0xFFFF;
    header.e_sp = 0x140;
    header.e_lfarlc = 0x40;
    header.e_lfanew = 0x80;

    write_bytes(writer, (const u8*)&header, sizeof(header));
    assert(writer->at == 0x40);
}

void write_dos_stub(exe_writer *writer) {
    const u8 dos_stub[] = {
        0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21,
        0xB8, 0x01, 0x4C, 0xCD, 0x21,

        'T', 'h', 'i', 's', ' ', 'p', 'r', 'o', 'g', 'r', 'a', 'm',
        ' ', 'c', 'a', 'n', 'n', 'o', 't', ' ', 'b', 'e', ' ', 'r',
        'u', 'n', ' ', 'i', 'n', ' ', 'D', 'O', 'S', ' ', 'm', 'o',
        'd', 'e', '.',

        0x0D, 0x0D, 0x0A, 0x24,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    write_bytes(writer, dos_stub, array_count(dos_stub));
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
    coff_header header;
    header.signature = FOURCC("PE\0\0");
    header.machine = IMAGE_FILE_MACHINE_AMD64;
    header.number_of_sections = num_sections;
    header.time_date_stamp = get_unix_time();
    header.pointer_to_symbol_table = 0; // deprecated
    header.number_of_sym_table = 0; // deprecated
    header.size_of_optional_header = 0;

    if(kind == pe_object_kind_exe) {
        if(format == pe_format_pe32_plus) {
            header.size_of_optional_header = 240;
        } else if(format == pe_format_pe32) {
            header.size_of_optional_header = 224;
        }
    } else if(kind == pe_object_kind_obj) {
        header.size_of_optional_header = 0;
    }

    header.characteristsics = IMAGE_FILE_EXECUTABLE_IMAGE    |
                              IMAGE_FILE_LARGE_ADDRESS_AWARE |
                              (has_debug_info ? 0 : IMAGE_FILE_DEBUG_STRIPPED);

    write_bytes(writer, (const u8*)&header, sizeof(coff_header));
}

void write_standard_coff_fields_32(exe_writer *writer,
                                   u32 size_of_code,
                                   u32 size_of_initialized_data,
                                   u32 size_of_uninitialized_data,
                                   u32 address_of_entry_point,
                                   u32 base_of_code,
                                   u32 base_of_data) {
    standard_coff_fields_32 coff_fields;
    coff_fields.magic = OPTIONAL_HEADER_PE32;
    coff_fields.major_linker_version = 0;
    coff_fields.minor_linker_version = 0;
    coff_fields.size_of_code = size_of_code;
    coff_fields.size_of_initialized_data = size_of_initialized_data;
    coff_fields.size_of_uninitialized_data = size_of_uninitialized_data;
    coff_fields.addr_of_entry_point = address_of_entry_point;
    coff_fields.base_of_code = base_of_code;
    coff_fields.base_of_data = base_of_data;

    write_bytes(writer, (const u8*)&coff_fields, sizeof(standard_coff_fields_32));
    assert(writer->at == 0x80 + 52);
}

void write_standard_coff_fields_64(exe_writer *writer,
                                   u32 size_of_code,
                                   u32 size_of_initialized_data,
                                   u32 size_of_uninitialized_data,
                                   u32 address_of_entry_point,
                                   u32 base_of_code) {
    standard_coff_fields_64 coff_fields;
    coff_fields.magic = OPTIONAL_HEADER_PE32_PLUS;
    coff_fields.major_linker_version = 0;
    coff_fields.minor_linker_version = 0;
    coff_fields.size_of_code = size_of_code;
    coff_fields.size_of_initialized_data = size_of_initialized_data;
    coff_fields.size_of_uninitialized_data = size_of_uninitialized_data;
    coff_fields.addr_of_entry_point = address_of_entry_point;
    coff_fields.base_of_code = base_of_code;
    write_bytes(writer, (const u8*)&coff_fields, sizeof(standard_coff_fields_64));
    assert(writer->at == 0x80 + 48);
}

u32 pe_checksum(mem_arena *arena, const u8 *buffer, u32 file_size, u32 checksum_field_offset) {
    u32 scratch_size = align_up_next_pow2(file_size, 2);
    u16 *scratch = (u16*)arena_push(arena, scratch_size, 1, 1);

    memory_copy(scratch, buffer, file_size);
    if(file_size & 1) {
        ((u8*)scratch)[file_size] = 0; // pad odd trailing byte with 0
    }
    *(u32*)((u8*)scratch + checksum_field_offset) = 0;

    u32 num_words = file_size / 2;
    u32 sum = 0;

    const u16 *word_ptr = scratch;
    for(u32 i = 0; i < num_words; i++) {
        sum += word_ptr[i];
        sum = (sum & 0xFFFF) + (sum >> 16); // fold carry back in
    }

    if(file_size & 1) {
        sum += ((u8*)scratch)[file_size - 1];
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Final fold, then add the file size, then fold once more.
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += file_size;

    return sum;
}

void write_data_directory(exe_writer *writer, b32 has_debug_info,
                          u32 import_dir_rva, u32 import_dir_size,
                          u32 iat_rva, u32 iat_size) {
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // export
    write_data_directory_entry(writer, (image_data_directory_entry){import_dir_rva, import_dir_size}); // import
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // resources
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // exception
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Certificates
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // base relocs
    image_data_directory_entry debug_dir;
    debug_dir.size = 0;
    debug_dir.virtual_address = 0;
    if(has_debug_info) {
        //TODO: put the correct values in here for debug builds.
        assert(0);
        debug_dir.size = 0;
        debug_dir.virtual_address = 0;
    }
    write_data_directory_entry(writer, debug_dir); // debug info
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Architecture (reserved, must be zero)
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Global Pointer Register (not a real thing on x64)
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Thread Local Storage
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Load Config Table
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Bound Import (for dlls, I think)
    write_data_directory_entry(writer, (image_data_directory_entry){iat_rva, iat_size}); // IAT
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Delay Import Desc (not used, I think)
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Common Language Runtime (CLR) header and size. I assume that this should be zero for us since we are not using the CLR.
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // must be zero
}

void write_windows_specific_fields_32(exe_writer *writer,
                                      u64 image_base,
                                      u32 section_align,
                                      u32 file_align,
                                      u32 size_of_headers,
                                      u32 size_of_image,
                                      u16 subsystem,
                                      b32 has_debug_info,
                                      u32 import_dir_rva, u32 import_dir_size,
                                      u32 iat_rva, u32 iat_size) {
    windows_specific_fields_32 fields;

    fields.ImageBase = image_base;
    fields.SectionAlignment = section_align;
    fields.FileAlignment = file_align;

    assert(fields.SectionAlignment >= fields.FileAlignment && "section alignment has to be greater than or equal to file alignment");

    // NOTE: I don't think the version actually matters.
    // also, we need to do some manifest bullshit in order
    // to get version 10 (current version)
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
    fields.MajorOperatingSystemVersion = 6;
    fields.MinorOperatingSystemVersion = 2;
    // os version
    fields.MajorImageVersion = 0;
    fields.MinorImageVersion = 0;

    fields.MajorSubsystemVersion = 6;
    fields.MinorSubsystemVersion = 2;

    fields.Win32VersionValue = 0; // reserved field
    fields.SizeOfImage = size_of_image;
    fields.SizeOfHeaders = size_of_headers;
    fields.CheckSum = 0; // TODO: Maybe we can ignore this.
                         // documentation says that only dlls and drivers get their checksum validated.

    fields.Subsystem = subsystem;
    fields.DllCharacteristics = 0; // TODO: fill this in for a .dll
    fields.SizeOfStackReserve = mebibytes(1);
    fields.SizeOfStackCommit = kibibytes(1);
    fields.SizeOfHeapReserve = mebibytes(1);
    fields.SizeOfHeapCommit = kibibytes(1);
    fields.LoaderFlags = 0; // apparently, this one is obsolete.
    // NOTE: Technically, this can vary. In practice, most people just put in
    // the maximum of 16 and they put in all zeroes for the image_data_directories
    // that they don't want to use
    fields.NumberOfRvaAndSizes = 16;

    write_bytes(writer, (const u8*)&fields, sizeof(windows_specific_fields_32));
    write_data_directory(writer, has_debug_info, import_dir_rva, import_dir_size, iat_rva, iat_size);

    // at this point, this is what the offset should be for a 32 bit executable
    assert(writer->at == 0x80 + sizeof(coff_header) + sizeof(standard_coff_fields_32) + sizeof(windows_specific_fields_32) + sizeof(image_data_directory_entry) * 16);
}

void write_windows_specific_fields_64(exe_writer *writer, u64 image_base,
                                      u32 section_align, u32 file_align,
                                      u32 size_of_headers, u32 size_of_image,
                                      u16 subsystem, b32 has_debug_info,
                                      u32 import_dir_rva, u32 import_dir_size,
                                      u32 iat_rva, u32 iat_size) {
    windows_specific_fields_64 fields;
    fields.ImageBase = image_base;
    fields.SectionAlignment = section_align;
    fields.FileAlignment = file_align;

    assert(fields.SectionAlignment >= fields.FileAlignment && "section alignment has to be greater than or equal to file alignment");

    // NOTE: The windows does not seem to care about all of these
    // version numbers.
    // Maybe this is helpful:
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
    fields.MajorOperatingSystemVersion = 6;
    fields.MinorOperatingSystemVersion = 2;
    // os version
    fields.MajorImageVersion = 0;
    fields.MinorImageVersion = 0;

    fields.MajorSubsystemVersion = 6;
    fields.MinorSubsystemVersion = 2;

    fields.Win32VersionValue = 0; // reserved field
    fields.SizeOfImage = size_of_image;
    fields.SizeOfHeaders = size_of_headers;
    fields.CheckSum = 0; // TODO: Maybe we can ignore this.
                         // documentation says that only dlls and drivers get their checksum validated.

    fields.Subsystem = subsystem;
    fields.DllCharacteristics = 0; // TODO: fill this in for a .dll
    fields.SizeOfStackReserve = mebibytes(1);
    fields.SizeOfStackCommit = kibibytes(1);
    fields.SizeOfHeapReserve = mebibytes(1);
    fields.SizeOfHeapCommit = kibibytes(1);
    fields.LoaderFlags = 0; // apparently, this one is obsolete.
    // NOTE: Technically, this can vary. In practice, most people just put in
    // the maximum of 16 and they put in all zeroes for the image_data_directories
    // that they don't want to use
    fields.NumberOfRvaAndSizes = 16;

    write_bytes(writer, (const u8*)&fields, sizeof(windows_specific_fields_64));
    write_data_directory(writer, has_debug_info, import_dir_rva, import_dir_size, iat_rva, iat_size);
    // at this point, this is what the offset should be for a 64 bit executable
    assert(writer->at == 0x80 + sizeof(coff_header) + sizeof(standard_coff_fields_64) + sizeof(windows_specific_fields_64) + sizeof(image_data_directory_entry) * 16);
}

u32 get_section_characteristics(string8 section_name) {
    if(str_match(section_name, s(".text"))) {
        return text_flags;
    } else if(str_match(section_name, s(".rodata"))) {
        return rodata_flags;
    } else if(str_match(section_name, s(".data"))) {
        return data_flags;
    } else if(str_match(section_name, s(".idata"))) {
        return idata_flags;
    } else if(str_match(section_name, s(".bss"))) {
        return bss_flags;
    }
    unreachable;
    return 0;
}

void write_image_section_header(exe_writer *writer, string8 section_name,
                                pe_object_kind kind, u32 file_align, u32 virtual_size,
                                u32 virtual_address, u32 size_of_raw_data, u32 pointer_to_raw_data,
                                u16 num_relocations, u32 characteristics) {
    char name[8];
    memset(name, 0, 8);
    memory_copy(name, section_name.data, section_name.length);
    write_u64(writer, *(u64*)&name);

    if(kind == pe_object_kind_obj) {
        virtual_size = 0;
    }

    write_u32(writer, virtual_size);
    write_u32(writer, virtual_address);

    // NOTE: I am not sure if this assumption is correct, let's comment it out for now.
    // It is correct according to the docs, but windows still loads the .exe and runs it just fine.
    // assert(size_of_raw_data % file_align == 0);
    write_u32(writer, size_of_raw_data);
    write_u32(writer, pointer_to_raw_data);

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

void write_import_dir_entry(exe_writer *writer, u32 import_lookup_rva, u32 time_date_stamp, u32 forwarder_chain, u32 name_rva, u32 thunk_table_rva) {
    import_dir_entry entry;
    entry.import_lookup_rva = import_lookup_rva;
    entry.time_date_stamp = time_date_stamp;
    entry.forwarder_chain = forwarder_chain;
    entry.name_rva = name_rva;
    entry.thunk_table_rva = thunk_table_rva;
    write_bytes(writer, (const u8*)&entry, sizeof(import_dir_entry));
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

void write_ilt_or_iat_entry_ordinal(exe_writer *writer, pe_format format, u32 hint_name_table_rva) {
    write_bits(writer, (u8*)&hint_name_table_rva, 31);

    u32 zeroes = 0;
    if(format == pe_format_pe32_plus) {
        write_bits(writer, (u8*)&zeroes, 32);
    }

    u8 ordinal_flag = 0;
    write_bits(writer, &ordinal_flag, 1);
}

void write_ilt_or_iat_entry(exe_writer *writer, pe_format format, u32 hint_name_table_rva) {
    // Format for pe32+ is: 31 bits for hint_name_table_rva, 32 bits of zero padding, and 1 bit for the ordinal flag.
    // For pe32, the format is: 31 bits for hint_name_table_rva, and 1 bit for the ordinal flag.
    // because we want to import by name, the ordinal flag is zero, which means that we can get away with writing this field as if it was just one value.

    if(format == pe_format_pe32_plus) {
        write_u64(writer, hint_name_table_rva);
    } else {
        write_u32(writer, hint_name_table_rva);
    }
}

u32 get_ilt_or_iat_entry_size(pe_format format) {
    if(format == pe_format_pe32_plus) {
        return 8;
    } else if(format == pe_format_pe32) {
        return 4;
    }
    unreachable;
}

u32 get_hint_name_table_entry_size(string8 function_name) {
    // Matches write_hint_name_table_entry's layout: u16 hint + name bytes +
    // null terminator, padded up to an even size.
    u32 size = 2 + function_name.length + 1;
    if(size % 2 != 0) {
        size += 1;
    }
    return size;
}

void write_ilt_and_iat(const import *imports, u32 import_count, exe_writer *idata_section_writer, pe_format format, u32 idata_section_rva) {
    u32 offset = idata_section_rva + idata_section_writer->at;
    u32 ilt_or_iat_entry_size = get_ilt_or_iat_entry_size(format);

    // The hint/name table starts right after the ILTs and the IATs.
    // Both tables together take 2 * (function_count + 1) entries per dll
    // (the +1 is the null terminator entry), regardless of ordering.
    for(u32 i = 0; i < import_count; i++) {
        const import *dll = &imports[i];
        offset += ilt_or_iat_entry_size * 2 * dll->function_count;
    }
    offset += ilt_or_iat_entry_size * 2 * import_count;

    // All ILTs first, then all IATs. Keeping the IATs contiguous lets the
    // IAT data directory entry describe them with one exact range. (With a
    // single dll this is byte-identical to the old interleaved layout.)
    u32 ilt_block_start = idata_section_writer->at;
    for(u32 i = 0; i < import_count; i++) {
        const import *dll = &imports[i];
        for(u32 j = 0; j < dll->function_count; j++) {
            write_ilt_or_iat_entry(idata_section_writer, format, offset);
            offset += get_hint_name_table_entry_size(dll->function_names[j]);
        }
        write_ilt_or_iat_entry(idata_section_writer, format, 0); // null entry
    }
    u32 ilt_block_size = idata_section_writer->at - ilt_block_start;

    // The IAT starts out byte-identical to the ILT -- the loader
    // overwrites the IAT copy with real function addresses at load
    // time, but until then both tables hold the same hint/name RVAs.
    // The IATs are laid out in the same per-dll order as the ILTs, so
    // the whole IAT block is one copy of the whole ILT block.
    write_bytes(idata_section_writer, &idata_section_writer->buffer[ilt_block_start], ilt_block_size);
}

section_info get_first_section_info(string8 section_name, u32 section_align,
                                    u32 file_align, const u8* section_data,
                                    u32 size_of_raw_data, u32 raw_headers_size) {
    section_info info;
    info.section_name = section_name;
    info.rva = align_up_next_pow2(raw_headers_size, section_align);
    info.virtual_size = align_up_next_pow2(size_of_raw_data, section_align);
    info.offset_on_disk = align_up_next_pow2(raw_headers_size, file_align);
    info.size_on_disk = align_up_next_pow2(size_of_raw_data, file_align);
    info.section_data = section_data;
    info.size_of_raw_data = size_of_raw_data;
    return info;
}

section_info get_next_section_info(string8 section_name,
                                   u32 section_align,
                                   u32 file_align,
                                   const u8 *section_data,
                                   u32 size_of_raw_data,
                                   section_info *prev_info) {
    section_info info;
    info.section_name = section_name;
    info.rva = align_up_next_pow2(prev_info->rva + prev_info->virtual_size, section_align);
    info.virtual_size = align_up_next_pow2(size_of_raw_data, section_align);
    info.offset_on_disk = align_up_next_pow2(prev_info->offset_on_disk + prev_info->size_on_disk, file_align);
    info.size_on_disk =  align_up_next_pow2(size_of_raw_data, file_align);
    info.section_data = section_data;
    info.size_of_raw_data = size_of_raw_data;
    return info;
}

u32 get_next_section_rva(u32 section_align, section_info *prev_info) {
    return align_up_next_pow2(prev_info->rva + prev_info->virtual_size, section_align);
}

void write_section_padding_bytes(exe_writer *writer, const section_info *info) {
    u32 num_padding_bytes = info->size_on_disk - info->size_of_raw_data;

    memset(&writer->buffer[writer->at], 0, num_padding_bytes);
    writer->at += num_padding_bytes;
}

u32 get_raw_headers_size(pe_format format, u32 num_sections) {
    u32 optional_header_size = 0;
    if(format == pe_format_pe32_plus) {
        optional_header_size = 240;
    } else if(format == pe_format_pe32) {
        optional_header_size = 224;
    } else {
        assert(0);
    }
    // dos header + stub (0x80) + coff header (24) + optional header + section table
    return 0x80 + 24 + optional_header_size + num_sections * 40;
}

section_list pe_get_section_infos(mem_arena *arena, generated_code *code,
                               u32 file_align, u32 section_align,
                               pe_format format) {

    u32 raw_headers_size = get_raw_headers_size(format, code->num_sections);

    section_info *infos = arena_push(arena, sizeof(section_info) * code->num_sections, alignof(section_info), 1);
    u32 idx = 0;

    if(code->size_of_text) {
        infos[idx] = get_first_section_info(s(".text"), section_align, file_align, code->text, code->size_of_text, raw_headers_size);
        idx++;
    }
    if(code->size_of_rodata) {
        infos[idx] = get_next_section_info(s(".rodata"), section_align, file_align, code->rodata, code->size_of_rodata, &infos[idx - 1]);
        idx++;
    }
    if(code->size_of_data) {
        infos[idx] = get_next_section_info(s(".data"), section_align, file_align, code->data, code->size_of_data, &infos[idx - 1]);
        idx++;
    }
    if(code->size_of_bss) {
        assert(0); // still not sure what to do about BSS.
    }

    u32 import_dir_rva = 0;
    u32 import_dir_size = 0;
    u32 iat_rva = 0;
    u32 iat_size = 0;

    if(code->import_count) {
        exe_writer idata_section_writer = writer_init(arena, mebibytes(10));
        const u32 idata_rva = get_next_section_rva(section_align, &infos[idx - 1]);
        const u32 entry_size = get_ilt_or_iat_entry_size(format);

        u32 size_of_directory_table = (code->import_count + 1) * 0x14; // +1 for null descriptor

        // The import descriptor table is the first thing in the section.
        import_dir_rva = idata_rva;
        import_dir_size = size_of_directory_table;

        u32 *dll_name_offset = arena_push(arena, sizeof(u32) * code->import_count, alignof(u32), 1);
        u32 running = size_of_directory_table;
        for(u32 i = 0; i < code->import_count; i++) {
            dll_name_offset[i] = running;
            running += code->imports[i].dll_name.length + 1;
        }
        u32 names_end = running;
        u32 padding = (8 - ((idata_rva + names_end) & 7)) & 7;
        u32 ilt_iat_start = names_end + padding;

        u32 *ilt_offset = arena_push(arena, sizeof(u32) * code->import_count, alignof(u32), 1);
        u32 *iat_offset = arena_push(arena, sizeof(u32) * code->import_count, alignof(u32), 1);
        running = ilt_iat_start;
        for(u32 i = 0; i < code->import_count; i++) {
            ilt_offset[i] = running;
            running += (code->imports[i].function_count + 1) * entry_size;
        }
        iat_rva = idata_rva + running;
        for(u32 i = 0; i < code->import_count; i++) {
            iat_offset[i] = running;
            u32 table_size = (code->imports[i].function_count + 1) * entry_size;
            running += table_size;
            iat_size += table_size;
        }

        for(u32 i = 0; i < code->import_count; i++) {
            write_import_dir_entry(&idata_section_writer,
                idata_rva + ilt_offset[i],
                0, 0,
                idata_rva + dll_name_offset[i],
                idata_rva + iat_offset[i]);
        }
        write_import_dir_entry(&idata_section_writer, 0, 0, 0, 0, 0);

        for(u32 i = 0; i < code->import_count; i++) {
            write_bytes(&idata_section_writer, code->imports[i].dll_name.data, code->imports[i].dll_name.length);
            write_u8(&idata_section_writer, 0);
        }

        u32 offset = idata_rva + idata_section_writer.at;
        u32 num_padding_bytes = (8 - (offset & 7)) & 7;
        for(u32 i = 0; i < num_padding_bytes; i++) {
            write_u8(&idata_section_writer, 0);
        }
        assert(idata_section_writer.at == ilt_iat_start &&
               "idata layout drifted from the precomputed offsets -- descriptors now point at the wrong bytes");

        write_ilt_and_iat(code->imports, code->import_count, &idata_section_writer, format, idata_rva);

        for(u32 i = 0; i < code->import_count; i++) {
            const import *dll = &code->imports[i];
            for (u32 j = 0; j < dll->function_count; j++) {
                write_hint_name_table_entry(&idata_section_writer, dll->function_names[j]);
            }
        }

        infos[idx] = get_next_section_info(s(".idata"), section_align, file_align, idata_section_writer.buffer, idata_section_writer.at, &infos[idx - 1]);
        idx++;
    }

    return (section_list) {.infos = infos, .num_sections = code->num_sections,
                           .import_dir_rva = import_dir_rva, .import_dir_size = import_dir_size,
                           .iat_rva = iat_rva, .iat_size = iat_size};
}

section_info *get_section_info(section_list *list, string8 section_name) {
    for(u32 i = 0; i < list->num_sections; i++) {
        section_info *info = &list->infos[i];
        if(str_match(section_name, info->section_name)) {
            return info;
        }
    }
    assert(0);
    return NULL;
}

void generate_pe_file(mem_arena *arena, string8 output_path, generated_code *code) {

    const u64 image_base = 0x140000000;
    const pe_format format = pe_format_pe32_plus;
    const pe_object_kind kind = pe_object_kind_exe;
    const u32 section_align = 4096;
    const u32 file_align = 512;
    const b32 is_gui = false;
    const b32 has_debug_info = false;

    section_list sec_list = pe_get_section_infos(arena, code, file_align, section_align, format);
    // idata
    exe_writer pe_writer = writer_init(arena, mebibytes(10));

    write_dos_header(&pe_writer);
    write_dos_stub(&pe_writer);

    write_coff_header(&pe_writer, format, kind, has_debug_info, sec_list.num_sections);

    u32 size_of_init_data = code->size_of_data + code->size_of_rodata;
    u32 size_of_uninit_data = code->size_of_bss;
    section_info *text_info = get_section_info(&sec_list, s(".text"));
    section_info *data_info = get_section_info(&sec_list, s(".data"));
    section_info *rodata_info = get_section_info(&sec_list, s(".rodata"));
    section_info *idata_info = get_section_info(&sec_list, s(".idata"));

    // NOTE: we are using the same value for the address of the entry point and
    // the start of the text section. which means that when we write out the
    // text section, the entry point has to be the first thing in the text
    // section.
    if(format == pe_format_pe32_plus) {
        write_standard_coff_fields_64(&pe_writer, code->size_of_text, size_of_init_data, size_of_uninit_data,
                                       text_info->rva, text_info->rva);
    } else if (format == pe_format_pe32) {
        write_standard_coff_fields_32(&pe_writer, code->size_of_text, size_of_init_data, size_of_uninit_data,
                                       text_info->rva, text_info->rva, data_info->rva);
    }

    u32 raw_headers_size = get_raw_headers_size(format, sec_list.num_sections);
    u32 size_of_headers = align_up_next_pow2(raw_headers_size, file_align);
    // size of image after it's been mapped into memory.
    u32 virtual_size_of_sections = text_info->virtual_size + rodata_info->virtual_size + data_info->virtual_size + idata_info->virtual_size;
    u32 size_of_image = size_of_headers + virtual_size_of_sections;
    u16 subsystem = is_gui ? IMAGE_SUBSYSTEM_WINDOWS_GUI : IMAGE_SUBSYSTEM_WINDOWS_CUI;
    if(format == pe_format_pe32_plus) {
        write_windows_specific_fields_64(&pe_writer, image_base, section_align, file_align,
                                         size_of_headers, size_of_image, subsystem, has_debug_info,
                                         sec_list.import_dir_rva, sec_list.import_dir_size,
                                         sec_list.iat_rva, sec_list.iat_size);
    } else {
        write_windows_specific_fields_32(&pe_writer, image_base, section_align, file_align,
                                         size_of_headers, size_of_image, subsystem, has_debug_info,
                                         sec_list.import_dir_rva, sec_list.import_dir_size,
                                         sec_list.iat_rva, sec_list.iat_size);
    }

    for(u32 i = 0; i < sec_list.num_sections; i++) {
        section_info *info = &sec_list.infos[i];
        u32 characteristics = get_section_characteristics(info->section_name);
        write_image_section_header(&pe_writer, info->section_name, kind, file_align, info->virtual_size, info->rva, info->size_of_raw_data, info->offset_on_disk, 0, characteristics);
    }

    for(u32 i = 0; i < sec_list.num_sections; i++) {
        section_info *info = &sec_list.infos[i];

        // Zero-fill any gap between where the writer currently is and where
        // this section's header says its raw data lives on disk. Without this,
        // section bytes get packed right after the headers while the section
        // headers point at later (empty) file offsets.
        assert(pe_writer.at <= info->offset_on_disk);
        u32 gap = info->offset_on_disk - (u32)pe_writer.at;
        memset(&pe_writer.buffer[pe_writer.at], 0, gap);
        pe_writer.at += gap;

        write_bytes(&pe_writer, info->section_data, info->size_of_raw_data);
        write_section_padding_bytes(&pe_writer, info);
    }


    if(!str_ends_with(output_path, s(".exe"))) {
        string_builder exe_path_builder = sb_create(arena);
        sb_append(&exe_path_builder, output_path);
        output_path = sb_append(&exe_path_builder, s(".exe"));
    }

    pal_write_file(pe_writer.buffer, pe_writer.at, str_to_cstr(arena, output_path));
}

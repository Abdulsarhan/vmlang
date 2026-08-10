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

#define Bit(x)((x) << 1)
#define binary_flag_has_text   Bit(0)
#define binary_flag_has_data   Bit(1)
#define binary_flag_has_rodata Bit(2)
#define binary_flag_has_bss    Bit(3)
#define binary_flag_has_idata  Bit(4)

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
    u64 size_of_code;
    u64 size_of_initialized_data;
    u64 size_of_uninitialized_data;
    u64 addr_of_entry_point;
    u64 base_of_code;
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

void write_data_directory_entry(exe_writer *writer, image_data_directory_entry directory) {
    if(writer->at + 8 < writer->buffer_size) {
        memcpy(&writer->buffer[writer->at], &directory, sizeof(image_data_directory_entry));
        writer->at += 8;
    } else {
        printf("not going to write");
    }
}

void write_dos_header(exe_writer *writer) {
    dos_header header;
    // NOTE: Most of the fields are typically set to 0, so we just do a memset
    // upfront instead of manually setting each field.
    memset(&header, 0, sizeof(header));
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

        0x2E, 0x0D, 0x0D, 0x0A, 0x24,
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
            header.size_of_optional_header = 240 + num_sections * 40;
        } else if(format == pe_format_pe32) {
            header.size_of_optional_header = 224 + num_sections * 40;
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
    assert(writer->at == 0x80 + 28);
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
    assert(writer->at == 0x80 + 24);
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

void write_data_directory(exe_writer *writer, b32 has_debug_info) {
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // export
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // import
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
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // IAT
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // Delay Import Desc (not used, I think)
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // CLR runtime header and size. I assume that this should be zero for us since we are not using the CLR.
    write_data_directory_entry(writer, (image_data_directory_entry){0, 0}); // must be zero
}

void write_windows_specific_fields_32(exe_writer *writer,
                                      u64 image_base,
                                      u32 section_align,
                                      u32 file_align,
                                      u32 size_of_headers,
                                      u32 size_of_image,
                                      u16 subsystem,
                                      b32 has_debug_info) {
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
    u32 checksum = 0; assert(0); // TODO: Maybe we can ignore this.
                                 // documentation says that only dlls and drivers get their checksum validated.
    fields.CheckSum = checksum;

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
    write_data_directory(writer, has_debug_info);

    // at this point, this is what the offset should be for a 32 bit executable
    assert(writer->at == 0x80 + 224);
}

void write_windows_specific_fields_64(exe_writer *writer, u64 image_base,
                                      u32 section_align, u32 file_align,
                                      u32 size_of_headers, u32 size_of_image,
                                      u16 subsystem, b32 has_debug_info) {
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
    u32 checksum = 0; assert(0); // TODO: Maybe we can ignore this.
                                 // documentation says that only dlls and drivers get their checksum validated.
    fields.CheckSum = checksum;

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
    write_data_directory(writer, has_debug_info);
    // at this point, this is what the offset should be for a 64 bit executable
    assert(writer->at == 0x80 + 240);
}

u32 get_section_characteristics(string8 section_name) {
    if(str_are_strings_equal(section_name, STR8_LIT(".text"))) {
        return text_flags;
    } else if(str_are_strings_equal(section_name, STR8_LIT(".rodata"))) {
        return rodata_flags;
    } else if(str_are_strings_equal(section_name, STR8_LIT(".data"))) {
        return data_flags;
    } else if(str_are_strings_equal(section_name, STR8_LIT(".idata"))) {
        return idata_flags;
    } else if(str_are_strings_equal(section_name, STR8_LIT(".bss"))) {
        return bss_flags;
    }
    // unreachable code
    assert(0);
    return 0;
}

void write_image_section_header(exe_writer *writer, string8 section_name,
                                pe_object_kind kind, u32 file_align, u32 virtual_size,
                                u32 virtual_address, u32 size_of_raw_data, u16 num_relocations,
                                u32 characteristics) {
    char name[8];
    memset(name, 0, 8);
    memcpy(name, section_name.data, section_name.length);
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

void write_ilt_or_iat_entry(exe_writer *writer, pe_format format, u32 hint_name_table_rva) {
    write_bits(writer, (u8*)&hint_name_table_rva, 31);

    u32 zeroes = 0;
    if(format == pe_format_pe32_plus) {
        write_bits(writer, (u8*)&zeroes, 32);
    }

    u8 ordinal_flag = 0;
    write_bits(writer, &ordinal_flag, 1);
}

u32 get_ilt_or_iat_entry_size(pe_format format) {
    if(format == pe_format_pe32_plus) {
        return 8;
    } else if(format == pe_format_pe32) {
        return 4;
    }
    assert(0); // unreachable code
}

void write_ilt_and_iat(const import *imports, u32 import_count, exe_writer *idata_section_writer, pe_format format, u32 idata_section_rva) {
    u32 offset = idata_section_rva + idata_section_writer->at;

    u32 ilt_or_iat_entry_size = get_ilt_or_iat_entry_size(format);
    for(u32 i = 0; i < import_count; i++) {
        const import *dll = &imports[i];
        offset += ilt_or_iat_entry_size * 2 * dll->function_count;
    }

    offset += ilt_or_iat_entry_size * 2 * import_count;
    u32 starting_offset = offset;

    for(u32 i = 0; i < import_count; i++) {
        const import *dll = &imports[i];
        for(u32 j = 0; j < dll->function_count; j++) {
            write_ilt_or_iat_entry(idata_section_writer, pe_format_pe32_plus, offset);
            offset += dll->function_names[j].length;
        }
        write_ilt_or_iat_entry(idata_section_writer, pe_format_pe32_plus, 0); // null entry

        offset = starting_offset;
        for(u32 j = 0; j < dll->function_count; j++) {
            write_ilt_or_iat_entry(idata_section_writer, pe_format_pe32_plus, offset);
            offset += dll->function_names[j].length;
        }
        write_ilt_or_iat_entry(idata_section_writer, pe_format_pe32_plus, 0); // null entry
        starting_offset = offset;
    }

}

section_info get_next_section_info(string8 section_name,
                                   u32 section_align,
                                   u32 file_align,
                                   const u8 *section_data,
                                   u32 size_of_raw_data,
                                   section_info *prev_info) {
    section_info info;
    info.section_name = section_name;
    info.rva = ALIGN_UP_NEXT_POW2(prev_info->rva + prev_info->virtual_size, section_align);
    info.virtual_size = ALIGN_UP_NEXT_POW2(size_of_raw_data, section_align);
    info.offset_on_disk = ALIGN_UP_NEXT_POW2(prev_info->offset_on_disk + prev_info->size_on_disk, file_align);
    info.size_on_disk =  ALIGN_UP_NEXT_POW2(size_of_raw_data, file_align);
    info.section_data = section_data;
    info.size_of_raw_data = size_of_raw_data;
    *prev_info = info;
    return info;
}

u32 get_next_section_rva(u32 section_align, section_info prev_info) {
    return ALIGN_UP_NEXT_POW2(prev_info.rva + prev_info.virtual_size, section_align);
}

void write_section_padding_bytes(exe_writer *writer, const section_info *info) {
    u32 num_padding_bytes = info->size_on_disk - info->size_of_raw_data;

    memset(&writer->buffer[writer->at], 0, num_padding_bytes);
    writer->at += num_padding_bytes;
}

typedef struct section_list section_list;
struct section_list {
    section_info *infos;
    u32 num_sections;
};

section_list get_section_infos(mem_arena *arena, const u8 *text, u32 size_of_text,
                               const u8 *data, u32 size_of_data, const u8 *rodata,
                               u32 size_of_rodata, u32 size_of_bss, const import *imports,
                               u32 import_count, u32 file_align, u32 section_align, pe_format format) {

    u32 num_sections = 0;

    section_info *infos = NULL;
    section_info prev_section;
    memset(&prev_section, 0, sizeof(prev_section));

    if(size_of_text) {
        num_sections++;
        infos = arena_push(arena, sizeof(section_info), 1, 1);
        *infos = get_next_section_info(STR8_LIT(".text"), section_align, file_align, text, size_of_text, &prev_section);
    }
    if(size_of_rodata) {
        num_sections++;
        infos = arena_push(arena, sizeof(section_info), 1, 1);
        *infos = get_next_section_info(STR8_LIT(".rodata"), section_align, file_align, rodata, size_of_rodata, &prev_section);
    }
    if(size_of_data) {
        num_sections++;
        infos = arena_push(arena, sizeof(section_info), 1, 1);
        *infos = get_next_section_info(STR8_LIT(".data"), section_align, file_align, data, size_of_data, &prev_section);
    }
    if(size_of_bss) {
        num_sections++;
        assert(0); // still not sure what to do about BSS.
    }
    if(import_count) {
        exe_writer idata_section_writer = writer_init(arena, mebibytes(10));
        const u32 idata_rva = get_next_section_rva(section_align, *(infos - num_sections));

        u32 size_of_directory_table = 0x14; // to account for null entry
        size_of_directory_table += import_count * 0x14;

        u32 import_lookup_table_offset = size_of_directory_table;
        for(u32 i = 0; i < import_count; i++) {
            import_lookup_table_offset += imports[i].dll_name.length;
        }

        u32 size_of_ilt_or_iat_entry = get_ilt_or_iat_entry_size(format);
        u32 size_of_first_ilt_entry = imports[0].function_count * size_of_ilt_or_iat_entry;
        u32 import_address_table_offset = import_lookup_table_offset + size_of_first_ilt_entry;

        for(u32 i = 0; i < import_count; i++) {
            write_import_dir_entry(&idata_section_writer, idata_rva + import_lookup_table_offset, 0, 0, idata_rva + 0x28, idata_rva + import_address_table_offset);
            u32 size_of_ilt_or_iat = imports[i].function_count * size_of_ilt_or_iat_entry;
            import_lookup_table_offset += size_of_ilt_or_iat;
            import_address_table_offset += size_of_ilt_or_iat;
        }
        write_import_dir_entry(&idata_section_writer, 0, 0, 0, 0, 0); // null descriptor

        for(u32 i = 0; i < import_count; i++) {
            const import *dll = &imports[i];
            for(u32 j = 0; j < dll->function_count; j++) {
                write_bytes(&idata_section_writer, dll->function_names[j].data, dll->function_names[j].length);
            }
        }

        u32 offset = idata_rva + idata_section_writer.at;
        u32 num_padding_bytes = (8 - (offset & 7)) & 7;

        for(u32 i = 0; i < num_padding_bytes; i++) {
            write_u8(&idata_section_writer, 0);
        }

        write_ilt_and_iat(imports, import_count, &idata_section_writer, format, idata_rva);

        for(u32 i = 0; i < import_count; i++) {
            const import *dll = &imports[i];
            for (u32 j = 0; j < dll->function_count; j++) {
                write_hint_name_table_entry(&idata_section_writer, dll->function_names[j]);
            }
        }

        num_sections++;
        infos = arena_push(arena, sizeof(section_info), 1, 1);
        *infos = get_next_section_info(STR8_LIT(".idata"), section_align, file_align, idata_section_writer.buffer, idata_section_writer.at - (u64)idata_section_writer.buffer, &prev_section);
    }
    return(section_list) {.infos = infos - num_sections, .num_sections = num_sections};
}

section_info *get_section_info(section_list *list, string8 section_name) {
    for(u32 i = 0; i < list->num_sections; i++) {
        section_info *info = &list->infos[i];
        if(str_are_strings_equal(section_name, info->section_name)) {
            return info;
        }
    }
    return NULL;
}

void generate_pe_file(ast_node *root, string8 output_path, const u8 *text,
                      u32 size_of_text, const u8 *data, u32 size_of_data,
                      const u8 *rodata, u32 size_of_rodata, u32 size_of_bss,
                      const import *imports, u32 import_count) {
    (void)root;

    const u64 image_base = 0x140000000;
    const pe_format format = pe_format_pe32_plus;
    const pe_object_kind kind = pe_object_kind_exe;
    const u32 section_align = 4096;
    const u32 file_align = 512;
    const b32 is_gui = true;
    const b32 has_debug_info = false;
    mem_arena *arena = arena_init(gibibytes(1));

    section_list sec_list = get_section_infos(arena, text, size_of_text,
                                              data, size_of_data, rodata,
                                              size_of_rodata, size_of_bss,
                                              imports, import_count, file_align,
                                              section_align, format);
    // idata
    exe_writer pe_writer = writer_init(arena, mebibytes(10));

    write_dos_header(&pe_writer);
    write_dos_stub(&pe_writer);

    write_coff_header(&pe_writer, format, kind, has_debug_info, sec_list.num_sections);

    u32 size_of_init_data = size_of_data + size_of_rodata;
    u32 size_of_uninit_data = size_of_bss;
    section_info *text_info = get_section_info(&sec_list, STR8_LIT(".text"));
    section_info *data_info = get_section_info(&sec_list, STR8_LIT(".data"));
    section_info *rodata_info = get_section_info(&sec_list, STR8_LIT(".rodata"));
    section_info *idata_info = get_section_info(&sec_list, STR8_LIT(".idata"));

    // NOTE: we are using the same value for the address of the entry point and
    // the start of the text section. which means that when we write out the
    // text section, the entry point has to be the first thing in the text
    // section.
    if(format == pe_format_pe32_plus) {
        write_standard_coff_fields_64(&pe_writer, size_of_text, size_of_init_data, size_of_uninit_data,
                                       text_info->rva, text_info->rva);
    } else if (format == pe_format_pe32) {
        write_standard_coff_fields_32(&pe_writer, size_of_text, size_of_init_data, size_of_uninit_data,
                                       text_info->rva, text_info->rva, data_info->rva);
    }

    u32 size_of_headers;
    if(format == pe_format_pe32_plus) {
        size_of_headers = 240;
    } else if(format == pe_format_pe32) {
        size_of_headers = 224;
    }
    size_of_headers += sec_list.num_sections * 40; // each section header is 40 bytes.
    // size of image after it's been mapped into memory.
    u32 virtual_size_of_sections = text_info->virtual_size + rodata_info->virtual_size + data_info->virtual_size + idata_info->virtual_size;
    u32 size_of_image = size_of_headers + virtual_size_of_sections;
    u16 subsystem = is_gui ? IMAGE_SUBSYSTEM_WINDOWS_GUI : IMAGE_SUBSYSTEM_WINDOWS_CUI;
    if(format == pe_format_pe32_plus) {
        write_windows_specific_fields_64(&pe_writer, image_base, section_align, file_align,
                                         size_of_headers, size_of_image, subsystem, has_debug_info);
    } else {
        write_windows_specific_fields_32(&pe_writer, image_base, section_align, file_align,
                                         size_of_headers, size_of_image, subsystem, has_debug_info);
    }

    for(u32 i = 0; i < sec_list.num_sections; i++) {
        section_info *info = &sec_list.infos[i];
        u32 characteristics = get_section_characteristics(info->section_name);
        write_image_section_header(&pe_writer, info->section_name, kind, file_align, info->virtual_size, info->rva, info->size_of_raw_data, 0, characteristics);
    }

    for(u32 i = 0; i < sec_list.num_sections; i++) {
        section_info *info = &sec_list.infos[i];
        write_bytes(&pe_writer, text, size_of_text);
        write_section_padding_bytes(&pe_writer, info);
    }

    pal_write_file(pe_writer.buffer, pe_writer.at - (u64)pe_writer.buffer, str_to_cstr(arena, output_path));
}

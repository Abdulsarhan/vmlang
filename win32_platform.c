#include "platform.h"
#include "ds.h"
#include <windows.h>

string8 read_entire_file(mem_arena *arena, string8 file_path) {
    string8 file = {0};

    const char *file_path_cstr = str_to_cstr(arena, file_path);
    HANDLE file_handle = CreateFileA(file_path_cstr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if(file_handle == INVALID_HANDLE_VALUE) {
        printf("failed to open file for reading.\n");
        return file;
    }

    LARGE_INTEGER file_size = {0};

    if(!GetFileSizeEx(file_handle, &file_size)) {
        printf("GetFileSizeEx failed.\n");
        CloseHandle(file_handle);
        return file;
    }

    file.length = file_size.QuadPart;
    file.data = arena_push_array_no_zero(arena, u8, file.length + 1); // for a null terminator

    // Because ReadFile only takes a u32 as a size for the file to read,
    // we have to call it in a loop to read files that are bigger than 2^32
    u64 total_bytes_read = 0;
    while(total_bytes_read < file.length) {
        u64 bytes_remaining = file.length - total_bytes_read;
        DWORD num_bytes_to_read = min(bytes_remaining, 0xFFFFFFFFull);
        DWORD bytes_read = 0;
        b32 err = ReadFile(file_handle, file.data + total_bytes_read, num_bytes_to_read, &bytes_read, 0);
        if(!err) {
            printf("ReadFile error: %lu\n", GetLastError());
            break;
        }
        if(bytes_read == 0) {
            break;
        }
        total_bytes_read += bytes_read;
    }

    CloseHandle(file_handle);

    file.data[file.length] = '\0';
    return file;
}

b32 write_entire_file(mem_arena *arena, string8 file_path, u8 *buffer, u64 buffer_size) {
    const char *file_path_cstr = str_to_cstr(arena, file_path);
    HANDLE file_handle = CreateFileA(file_path_cstr, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file_handle == INVALID_HANDLE_VALUE) {
        printf("failed to open file for writing.\n");
        return 0;
    }

    u64 total_bytes_written = 0;

    b32 success = 1;
    while(total_bytes_written < buffer_size) {
        u64 bytes_remaining = buffer_size - total_bytes_written;
        DWORD num_bytes_to_write = min(bytes_remaining, 0xFFFFFFFFull);
        DWORD bytes_written = 0;
        BOOL ok = WriteFile(file_handle, buffer + total_bytes_written, num_bytes_to_write, &bytes_written, NULL);
        if(!ok) {
            printf("WriteFile error: %lu\n", GetLastError());
            break;
        }
        total_bytes_written += bytes_written;
    }

    CloseHandle(file_handle);
    return success && (total_bytes_written == buffer_size);
}

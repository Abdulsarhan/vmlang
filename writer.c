#include "writer.h"

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

format pe64 console

entry start

include 'win64ax.inc'

; RCX, RDX, r8, R9, stack

STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE = -11

section '.text' code readable executable

    start:
        ; 32 bytes shadow space + 8 bytes for the 5th param = 0x28,
        ; which is already 16-byte aligned relative to entry (entry RSP is off by 8)
        sub rsp, 0x28
        mov ecx, STD_OUTPUT_HANDLE   ;nStdHandle
        call [GetStdHandle]
        mov rcx, rax                ;hConsoleOutput (in)
        lea rdx, [hello_world]      ;lpBuffer (in)
        mov r8d, hello_world_len    ;nNumberOfCharsToWrite (in)
        lea r9, [bytes_written]     ;lpBNumberOfCharsWritten (out, optional)
        mov qword [rsp + 0x20], 0   ;lpReserved (has to be zero)
        call [WriteConsoleA]

        xor ecx, ecx
        call [ExitProcess]

section '.rodata' data readable
hello_world db 'Hello from assembly!'
hello_world_len = $ - hello_world

section '.data' data readable writeable
bytes_written dd ?

section '.idata' import readable writable
    library KERNEL32, 'KERNEL32.dll'
    import KERNEL32,\
    ExitProcess, 'ExitProcess',\
    WriteConsoleA, 'WriteConsoleA',\
    GetStdHandle, 'GetStdHandle'

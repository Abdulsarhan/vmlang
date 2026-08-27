#define CB_IMPLEMENTATION
#include "cb.h"

int main(int argc, char **argv) {
    cb_rebuild_yourself(argc, argv);

    command cmd = {0};
    cb_append(&cmd, CB_STR8_LIT("clang -g -o vmlc.exe vmlc.c -std=c99 -lkernel32 -luser32 -ladvapi32 -lgdi32 -lshell32 -lcomdlg32 -lwinmm"));
    cb_append(&cmd, CB_STR8_LIT("-Wall -Wextra -Wimplicit-function-declaration -Wno-switch -Wno-unused-function"));
    cb_run(&cmd);
    return 0;
}

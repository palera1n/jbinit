#include <fakedyld/fakedyld.h>

static int gfd_console;

void _putchar(char character) {
    static size_t counter = 0;
    static char printbuf[0x100];
    printbuf[counter++] = character;
    if (character == '\n' || counter == sizeof(printbuf)) {
        write(gfd_console, printbuf, counter);
        counter = 0;
    }
}

void set_fd_console(int fd_console) {
    gfd_console = fd_console;
}

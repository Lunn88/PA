#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    size_t f_size = fs_filesz(fd);
    fs_read(fd, DEFAULT_ENTRY, f_size);
    fs_close(fd);
    return (uintptr_t)DEFAULT_ENTRY;
}

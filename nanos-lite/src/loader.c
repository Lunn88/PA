#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);

void *new_page();
uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  size_t f_size = fs_filesz(fd);
  void *fz_end = DEFAULT_ENTRY + f_size;
  void *va, *pa;
  for(va = DEFAULT_ENTRY; va < fz_end; va += PGSIZE){
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(fd, pa, (fz_end - va) < PGSIZE ? (fz_end - va) : PGSIZE);
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}

#include "common.h"

extern _RegSet* do_syscall(_RegSet* r);

static _RegSet* do_event(_Event e, _RegSet* r) {
printf("%d\n", _EVENT_SYSCALL);
    switch (e.event) {
        case _EVENT_SYSCALL:
            do_syscall(r); 
            break;
        default: panic("Unhandled event ID = %d", e.event);
    }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}

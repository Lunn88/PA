#include "trap.h"

char buf[128];

int main() {
	sprintf(buf, "%s", "Hello world!\n");
	nemu_assert(0 == 0);

	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	nemu_assert(0 == 0);

	sprintf(buf, "%d + %d = %d\n", 2, 10, 12);
	nemu_assert(0 == 0);

	return 0;
}

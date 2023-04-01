#include "trap.h"

char buf[128];

int main() {
	sprintf(buf, "%s", "Hello world!\n");
	printf("%d\n", strcmp(buf, "Hello world!\n")==0);
	nemu_assert(strcmp(buf, "Hello world!\n") == 0);

	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	nemu_assert(strcmp(buf, "1 + 1 = 2\n") == 0);

	sprintf(buf, "%d + %d = %d\n", 2, 10, 12);
	printf("%s\n", buf);
	nemu_assert(strcmp(buf, "2 + 10 = 12\n") == 0);

	return 0;
}

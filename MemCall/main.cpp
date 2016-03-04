#include <stdio.h>

const unsigned int a = 0x8BEC8B55;  /*  Эти значения нужно скорректировать */
const unsigned int b = 0x8BEC8B55;  /*  в зависимости от  */
const unsigned int c = 0x8BEC8B55;  /*  вывода программы. */
const unsigned int d = 0x8BEC8B55;



//0x00003A47
//0x003AE2E9
//0x054DE900

unsigned int func1(unsigned int i) {
	return i + 8;
}

int main(void) {
	unsigned int * i;
	unsigned int num;
	/*  Print out the bytecode for 'func1' */
	for (i = (unsigned int*)func1; i < (unsigned int*)main; i++) {
		printf("%p: 0x%08X\n", (void *)i, *i);
	}
	/*  Cast the address of 'a' to a function pointer and call it */
	num = ((unsigned int(*)(unsigned int))&a)(29);
	printf("%u\n", num); /* Выводит 37*/
}
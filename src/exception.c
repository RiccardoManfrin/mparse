#include <stdio.h>
#include "exception.h"

#ifdef EXCEPTION
void exception(char *filename, int line, exception_type x){
	printf("%s: %5u:\t### EXCEPTION %i\n", filename, line, x);
}
#endif

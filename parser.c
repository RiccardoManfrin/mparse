#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "src/parser.h"

void elem_free(void * obj){
	free(obj);
}

void elem_print(void * obj){
	printf("%s  \n",(char*)obj);
}

#define INIT_TIMERS() struct timeval start,stop, result
#define MEASURE_TIME(x) do{\
	gettimeofday(&start, NULL);\
	x;\
	gettimeofday(&stop, NULL);\
	timersub(&stop, &start, &result);\
	printf("TIME = %i,%05i\n", (int) result.tv_sec, (int) result.tv_usec);\
	}while(0)
	

		//Time meeasuring
// 				
// 		gettimeofday(&start, NULL);
// 		gettimeofday(&stop, NULL);
// 		timersub(&stop, &start, &result);
// 		printf("Expanding time = %i,%05i\n", (int) result.tv_sec, (int) result.tv_usec);

int main(int args, char *argv[])
{
 	//Parser tree generation test
	if(args-1==1)
	{

// 		INIT_TIMERS();
		mparser_t::init();
		mparser_t *p = new mparser_t();
		p->function((char *)"f(a)",(char *)"cos(a)");
		p->expression(argv[1]);
		parser_val_t res = p->calculate(); 
		printf("result = %f\n", res);
		delete p;
	}
	return 0;
}

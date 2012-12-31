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
		
// 		//Generating new parser
// 		parser_t *p = parser_new();
// 		//Defining function
// 		parser_func(p,"f(a)","cos(a)");
// 		//Defining a variable
// 		parser_var(p,"x", 4);
// 		
// 
// 		parser_config(p,argv[1]);
// 		
// 		//Calculating result
// 		//MEASURE_TIME(res = parser_calc(p,argv[1]));
// 		parser_val_t res = parser_calc(p); 
// 		printf("result = %f\n", res);
// 		
// // 		//Changing variable
// // 		int i; for(i=0;i<100;i++)
// // 		{
// // 			parser_var(p,"x", i);
// // 			//Calculating result
// // 			//MEASURE_TIME(res = parser_calc(p,argv[1]));
// // 			res = parser_calc(p,"f(x/100)");
// // 			printf("%f\n", res);
// // 		}
// 		parser_destroy(p);
	}
	return 0;
}

/** 
 * @author Riccardo Manfrin [namesurname at gmail dot com]
 * 
 * @brief  Debug features for verbose output
 * 
 */
#include <stdlib.h>
#include <stdarg.h>   /* handler for multiple arguments functions */
#include <string.h>
#include <sys/time.h> /* gettimeofday, timeval */
#include "debug.h"

typedef enum {
  INT,
  STR,
  CHR
} dbg_type;

typedef struct argnode{
  void *data;
  dbg_type t;
  struct argnode *next;
} arglist;

static char verbose = 0;

void dbg_verbose(int lev){
	verbose=lev;
}

unsigned int dbg_isverbose(int lev){
	if(verbose<=lev) return 1;
	return 0;
}

static char localize = 0;

void dbg_printloc(){
	localize=1;
}

void dbg_hideloc(){
	localize=0;
}

static char timestamp = 0;

void dbg_printtime(){
	timestamp=1;
}

void dbg_hidetime(){
	timestamp=0;
}

unsigned int dbg_islocalized(){
	return localize;
}

unsigned int dbg_istimestamp(){
	return timestamp;
}

void dbg_time_print(int lev){
	if(dbg_isverbose(lev))
	{
		struct timeval t;
		gettimeofday(&t,NULL);
		printf("[%i:%06i]\t", (int)t.tv_sec, (int)t.tv_usec);
	}
}

void dbg_loc_print(int lev, char *file, int line){
	if(dbg_isverbose(lev))
	{
		printf("%s: %5u:\t", file, line);
	}
	return;
}

void dbg_data_print(int lev, const void *str, ...){
  if(dbg_isverbose(lev)){
    va_list ap;
    int lg=strlen((char*)str);
    arglist lst;
    arglist *lst_ptr = &lst, *lst_ptr_ph;
    int lst_len = 0;
    int ctr;
    va_start(ap, str);
    
    for(ctr=0;ctr<lg;ctr++){
      if(((char*)str)[ctr]=='%')
      {
        switch(((char*)str)[ctr+1])
        {
          case 's':
          {
            lst_len++;
            lst_ptr->next = malloc(sizeof(arglist));
            memset(lst_ptr->next,0,sizeof(arglist));
            lst_ptr->next->t = STR;
            //Filling data
            lst_ptr->next->data = (char *) va_arg(ap, char *);
            
            lst_ptr=lst_ptr->next;
            break;
          }
          case 'c':
          {
            lst_len++;
            lst_ptr->next = malloc(sizeof(arglist));
            memset(lst_ptr->next,0,sizeof(arglist));
            lst_ptr->next->t = CHR;
            //Filling data
            lst_ptr->next->data = (char *) va_arg(ap, char *);
            lst_ptr=lst_ptr->next;
            break;
          }
          case 'i':
          {
            lst_len++;
            lst_ptr->next = malloc(sizeof(arglist));
            memset(lst_ptr->next,0,sizeof(arglist));
            lst_ptr->next->t = INT;
            //Filling data
            lst_ptr->next->data = (void*) va_arg(ap, int);
            lst_ptr=lst_ptr->next;
            break;
          }
          default:
          {
            break;
          }
        } // End of list setup
      }
    }
    ctr=0;
    lst_ptr=lst.next;
    while(((char*)str)[ctr]!=0){
      if(((char*)str)[ctr]!='%')
      {
        printf("%c", ((char*)str)[ctr]);
      }
      else
      {
        switch(lst_ptr->t){
          case (INT):
          {
            printf("%u",((int)lst_ptr->data));
            ctr++;
            break;
          }
          case (STR):
          {
            printf("%s",(char*)lst_ptr->data);
            ctr++;
            break;
          }
          case (CHR):
          {
            printf("%c",*((char*)lst_ptr->data));
            ctr++;
            break;
          }
          default:
          {
            break;
          }
        }
        lst_ptr_ph = lst_ptr;
        lst_ptr=lst_ptr->next;
		  free(lst_ptr_ph);
      }
      ctr++;
    }//EOWhile
    va_end(ap);
  }//EOdbg_is_verbose
  return;
}

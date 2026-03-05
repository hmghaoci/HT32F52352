#ifndef __KEY_H
#define __KEY_H 			   
#include "ht32.h"


#define WaitStatus     0 
#define PressStatus    1
#define ReleaseStatus  2
#define IDEStatus      3
#define KeyTime       800 

#define KEYPRESS       1
#define KEYRELEASE     2
#define KEYLONG        3

void key_init(void);
void keyscan(void);
unsigned char getkey(void);

#endif




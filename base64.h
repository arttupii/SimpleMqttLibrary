#ifndef __BASE__H_
#define  __BASE__H_
//#include<Arduino.h>
typedef unsigned char uint8_t;

bool toBase64(char *buf, int bufSize, const uint8_t* input, int inputSize);
int fromBase64(uint8_t *buf, int bufSize, const char* input);

#endif

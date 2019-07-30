#include "base64.h"
const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char toBase64Char(unsigned char v) {
    return base64chars[v];
}

int toBinaryFromBase64(char v) {
    if(v=='=') return -1;
    for(int i=0;i<sizeof(base64chars);i++){
        if(base64chars[i]==v) {
           return i;
        }
    }
    return -1;
}

bool toBase64(char *buf, int bufSize, const uint8_t* input, int inputSize) {
    int bufIndex=0;
    uint8_t tmp=0;
    char b=0;
    #define PUT_BUFFER_AND_CHECK(V) buf[bufIndex]=V; if(++bufIndex>=bufSize)return false;
    for(int i=0;i<inputSize;i++){
        for(char x=7;x>=0;x--) {
            tmp|=((input[i]&(1<<x))>>x)<<(5-b);
            b++;
            if(b>5){
                PUT_BUFFER_AND_CHECK(toBase64Char(tmp));
                tmp=0;
                b=0;
            }
        }
    }
    if(b!=0){
       buf[bufIndex]=toBase64Char(tmp);
       bufIndex++;
       if(bufIndex>bufSize) return false;
       for(int i=0;i<5-b;i++){
            PUT_BUFFER_AND_CHECK('=');
        }
    }
    PUT_BUFFER_AND_CHECK(0);
    #undef PUT_BUFFER_AND_CHECK
    return true;
}

int fromBase64(uint8_t *buf, int bufSize, const char* input) {
    char b=0;
    int index=0;
    for(int i=0;input[i]!=0;i++){
        int tmp = toBinaryFromBase64(input[i]);
        if(tmp<0) {
            return index;
        }
        if(b==0){
            b=6;
            buf[index]=tmp;
        } else if(b==6) {
            buf[index]=buf[index]<<2 | (tmp&0b110000)>>4;
            if(++index>=bufSize) return -1;
            buf[index]=(tmp&0b001111);
            b=4;
        } else if(b==4) {
            buf[index]=buf[index]<<4 | (tmp&0b111100)>>2;
            if(++index>=bufSize) return -1;
            buf[index]=(tmp&0b000011);
            b=2;
        } else if(b==2) {
            buf[index]=buf[index]<<6 | (tmp&0b111111);
            if(++index>=bufSize) return -1;
            b=0;
        }
    }
    return index;
}

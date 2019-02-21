#include<stdio.h>
#include<stdlib.h>
int main () {
    printf("%d %d %d\n", strtol("-123",NULL,8), strtol("-0123",NULL,10), strtol("-0x123",NULL,16));
    return 0;
}
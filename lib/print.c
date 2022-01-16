#include "print.h"
#include "sbi.h"

void puts(char *s) {
    while (*s != '\0')
    {
        sbi_ecall(0x1, 0x0, *s, 0, 0,  0, 0, 0);
        s++;
    }
    
}

void puti(int x) {
    if ( x == 0) {
        sbi_ecall(0x1, 0x0, '0', 0, 0,  0, 0, 0);
        return ; 
    }
    if(x < 0) {
        sbi_ecall(0x1, 0x0, '-', 0, 0,  0, 0, 0);
        x = -1 * x;
    }
    int temp = 1;
    while (temp * 10 < x) {
        temp *= 10;
    }
    while(temp > 0) {
        int out = x / temp;
        char out_char = out - 0 + '0';
        sbi_ecall(0x1, 0x0, out_char, 0, 0,  0, 0, 0);
        x = x - out * temp;
        temp = temp / 10;
    }
    return ;

}

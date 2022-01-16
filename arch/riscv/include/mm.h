#include "types.h"
#ifndef _mm_h__
#define _mm_h__
struct run {
    struct run *next;
};

void mm_init();

uint64 kalloc();
void kfree(uint64);

#endif
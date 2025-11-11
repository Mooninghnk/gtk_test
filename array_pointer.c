#include "stdio.h"

int main(void) {
    int a[] = {11, 22, 33, 44, 55};

    int *p = a;
    //array of chars
    char *x[] = {"hello", "world", "this", "is", "a", "test"};

//all there are the same but done in diffreent ways and some use a pointer p that just points to a;
//
    for (int i = 0; i < 5; i++)
        printf("A = %d\n", a[i]);

    for (int i = 0; i < 5; i++)
        printf("P = %d\n", p[i]);

    for (int i = 0; i < 5; i++)
        printf("a pointer notaion: %d\n", *(a + 1));

    for (int i = 0; i < 5; i++)
        printf("p pointer notation:%d\n", *(p + i));

    for (int i = 0; i < 5; i++)
        printf("MOVING p pointer notation:%d\n", *(p++));


    return 0;
}

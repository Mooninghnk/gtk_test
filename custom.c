#include <stdio.h>

void add(void *c, void *b) {
    int *n = c;
    int *d = b;
    int sum = *n + *d;
    printf("Sum: %d\n", sum);

}


int main(void) {
    int a = 5;
    int b = 10;
    add(&a, &b);
}

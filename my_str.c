#include <stdio.h>

int my_strlen(char *s) {
    char *p = s;
    while (*p != '\0') {
        p++;
        printf("%d", *p);
    }
    printf("x: %p\n", (void*)p);
    return p - s;
}
int main(void ) {
    char str[] = "Hello, World!";
    printf("Length of string: %d\n", my_strlen(str));
    return 0;
}

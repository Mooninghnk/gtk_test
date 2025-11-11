#include <stdio.h>

int main() {
    FILE *fp;
    int x = 32;

    fp = fopen("output.txt", "w");

    fputc('B', fp);
    fputc('\n', fp);
    fprintf(fp, "x = %d\n", x);
    fputs("hello world\n",fp);
    fclose(fp);
    return 0;
}

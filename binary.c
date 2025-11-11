#include <stdio.h>

int main(void) {
    FILE *fp;
    unsigned char bytes[6] = {5, 37, 123, 45, 67, 89};
    fp = fopen("output.bin", "wb");
    fwrite(bytes, sizeof(char), 6, fp);
    fclose(fp);

    return 0;

}

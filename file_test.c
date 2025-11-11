#include <stdio.h>

int main() {
  FILE *fp;
  char name[1024];
  float lenght;
  int mass;

  fp = fopen("./test.txt", "r");
  if (fp == NULL) {
    printf("Error opening file\n");
    return 1;
  }

  int c = fgetc(fp);
  if (c == EOF) {
    printf("Error reading file\n");
    fclose(fp);
    return 1;
  }
  while(fscanf(fp,"%s %f %d", name, &lenght, &mass) != EOF) {
      printf("Name: %s, Length: %f, Mass: %d\n", name, lenght, mass);
  }

  fclose(fp);
  return 0;
}

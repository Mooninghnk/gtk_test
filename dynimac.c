#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int *data;
  int size;     // How many elements we have
  int capacity; // How many elements we can hold
} DynamicArray;

DynamicArray *create_array(int inital_capacity) {
  DynamicArray *arr = malloc(sizeof(DynamicArray));
  arr->data = malloc(inital_capacity * sizeof(int));
  arr->size = 0;
  arr->capacity = inital_capacity;
  return arr;
}

void add_elememt(DynamicArray *arr, int value) {
  if (arr->size == arr->capacity) {
    arr->capacity *= 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(int));
  }
  arr->data[arr->size] = value;
  arr->size = arr->size + 1;
}

void destroy_array(DynamicArray *arr) {
  free(arr->data);
  free(arr);
}

int main(void) { DynamicArray *arr = create_array(3); }

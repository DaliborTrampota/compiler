#include <stdio.h>

// Runtime function for your language
void print_int(int x) {
    printf("%d\n", x);
}

// Add more runtime functions as needed
void print_double(double x) {
    printf("%f\n", x);
}

void print_char(char c) {
    putchar(c);
    putchar('\n');
}

void print_string(char* s) {
    printf("%s\n", s);
}


#include <raylib.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>

int sub(int x, int  y);
void voidFunction();
int add(int x, int  y);


int sub(int x, int  y) {
    return x - y;
}


void voidFunction() {

}

float x = 29; // TODO here make it 29.0;

char* st = "ciao";


#define COMPTIME 10;

typedef struct {
    int x;
    int y;
} Point;


typedef struct Value {
    bool b;
    Value v;
} Value;



int integer = 29;
float floating = 29.0;
char* string = "text";
char character = 'a';
bool boolean = true;
bool anotherBoolean = false;

int arr[] = {1, 2, 3};

char a[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
int n[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
float f[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0};

float ifarr[] = {1.0, 1.0, 2.0};
int tiiarr[3] = {6.0, 7.0, 8.0};
int tiiarrr[3] = {6.0, 7.0, 8.0};
int iarr[] = {3.0, 4.0, 5.0};
float farr[3] = {3.0, 4.0, 5.0};
char* sarr[2] = {"two", "strings"};
char carr[2] = {'a', 'b'};
bool barr[2] = {true, false};


char uarr[10] = {0};
bool anarr[10] = {0};
int arrr[3] = {1, 2, 3}; // C syntax always works



int main() {
    int variable = sub(2, 2); // NOTE This works somehow;
    int x = 10;


    if (x > 3) {
        printf("Hello World\n");
    }

    
    while (1) {
        printf("something\n");
        break;
    }

    
    sub(3, -1); // ANOTHER
    // Comment
    return 0;
    printf("Hello!\n"); // TODO Defers should pop before a return statement
}


int add(int x, int  y) {
    return x + y;
}


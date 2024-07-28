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


int a = 29;
float b = 29.0;
char* s = "text";
char c = 'a';
bool d = true;
bool e = false;



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
    printf("Hello!\n");
    return 0;
}


int add(int x, int  y) {
    return x + y;
}


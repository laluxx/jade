#include <raylib.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int sub(int x, int  y);
void voidFunction();
int add(int x, int  y);


int sub(int x, int  y) {
    return x - y;
}


void voidFunction() {

}

float x = 29;

#define COMPTIME 10;

typedef struct {
    float x;
    float y;
    int z;
} Vec3i;


int a = 29;
float b = 29.0;
char* s = "text";
char c = 'a';


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
    x = 0;
}


int add(int x, int  y) {
    return x + y;
}


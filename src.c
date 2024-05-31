#include <raylib.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int sub(int x, int  y);
int add(int x, int  y);


int sub(int x, int  y) {
    return x - y;
}


int main() {
    int x = 10;
    add(10, 5);
    sub(3, -1); // ANOTHER
    // Comment
    printf("Hello!\n");
    return 0;
}


int add(int x, int  y) {
    return x + y;
}


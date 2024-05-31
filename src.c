#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

int sub(int x, int  y);
int add(int x, int  y);

int sub(int x, int  y) {
    return x - y;
}

int main() {
    add(10, 5);
    sub(3, -1); // ANOTHER
    printf("Hello!\n");
    // Comment
    return 0;
}

int add(int x, int  y) {
    return x + y;
}

#include <stdio.h>
#include <stdlib.h>

int foo = 0x37;
int bar = 34;

int main() {
    long i;
    printf("\nHello\n\n");
    
    for (i=0; i<50000; i++) {
        printf("%d\n", i);
        foo = 0x37;
        printf("foo: (0x%x) = 0x%x\n", &foo, foo);
        bar = 34;
        printf("bar: (0x%x) = %d\n\n", &bar, bar);
    }
    
    return 0;
}

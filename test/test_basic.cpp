//
// Created by schrodinger on 2/4/22.
//

#include <memcpy-amd64/memcpy.h>
#include <cstdlib>

int main() {
    int x = 123;
    int y = 0;
    MEMCPY_AMD64_SYMBOL(memcpy)(&y, &x, sizeof(int));
    if (x != y) { ::abort(); }
    return 0;
}

#define MARKC_AUTO_COLLECT
#include "markc.h"

mark gc = {};

void x() { var j = new (ull, 102); }

int main(int argc, string *argv) {
    markc_start(&gc, &argc);

    ull last  = rdtsc();
    ull total = 0;
    ull count = 0;

    for (ull i = 0; i < 1000; i++) { var h = new (int, i); }
    printf("Hi\n");

    // printf("Avg: %llu Yo z %u Yo x %s Yo y %i\n", gc.avg_alloc_delay, *z, *x, *y);

    markc_stop(&gc);
    return 0;
}

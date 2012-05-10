#include "set.h"
#include <stdlib.h>

static int compare_ints(void *a, void *b)
{
    int *ia = a;
    int *ib = b;

    return (*ia)-(*ib);
}

static void *newint(int i)
{
    int *p = malloc(sizeof(int));
    *p = i;
    return p;
}

/*
 * Print a set of int elements.
 */
static void printset(char *prefix, set_t *set)
{
    set_iter_t *it;

    printf("%s", prefix);
    it = set_createiter(set);
    while (set_hasnext(it)) {
	    int *p = set_next(it);
	    printf(" %d", *p);
    }
    printf("\n");
    set_destroyiter(it);
}

/*
 * Print a set of int elements, then destroy it.
 */
static void dumpset(char *prefix, set_t *set)
{
    printset(prefix, set);
    set_destroy(set);
}

int main(int argc, char **argv)
{
    set_t *all, *evens, *odds, *nonprimes, *primes;
    int i, j, n = 50;
    int **numbers;

    /* Allocate numbers from 0 to n */
    numbers = (int **) malloc(sizeof(int) * (n+1));
    for (i = 0; i <= n; i++) {
	    numbers[i] = newint(i);
    }

    /* Create sets */
    all = set_create(compare_ints);
    evens = set_create(compare_ints);
    odds = set_create(compare_ints);
    nonprimes = set_create(compare_ints);
    primes = set_create(compare_ints);

    /* Initialize sets */
    for (i = 0; i <= n; i++) {
		set_add(all, numbers[i], numbers[i]);
	    if (i % 2 == 0) {
	        set_add(evens, numbers[i], numbers[i]);
	    }
	    else {
	        set_add(odds, numbers[i], numbers[i]);
	    }
        if (i < 2) {
            set_add(nonprimes, numbers[i], numbers[i]);
        }
        else {
	        for (j = i+i; j <= n; j += i) {
	            set_add(nonprimes, numbers[j], numbers[j]);
	        }
        }
	    if (!set_contains(nonprimes, numbers[i])) {
	        set_add(primes, numbers[i], numbers[i]);
	    }
    }

    /* Show resulting sets */
    printset("Numbers:", all);
    printset("Even numbers:", evens);
    printset("Odd numbers:", odds);
    printset("Non-prime numbers:", nonprimes);
    printset("Prime numbers:", primes);
    
    /* Test unions */
    dumpset("Even or odd numbers:", set_union(evens, odds));
    dumpset("Prime or non-prime numbers:", set_union(primes, nonprimes));
    dumpset("Even or prime numbers:", set_union(evens, primes));
    dumpset("Odd or prime numbers:", set_union(odds, primes));

    /* Test intersections */
    dumpset("Even and odd numbers:", set_intersection(evens, odds));
    dumpset("Even non-prime numbers:", set_intersection(evens, nonprimes));
    dumpset("Odd non-prime numbers:", set_intersection(odds, nonprimes));
    dumpset("Odd prime numbers:", set_intersection(odds, primes));
    dumpset("Even prime numbers:", set_intersection(evens, primes));

    /* Test differences */
    dumpset("Even non-prime numbers:", set_difference(evens, primes));
    dumpset("Odd non-prime numbers:", set_difference(odds, primes));
    dumpset("Even prime numbers:", set_difference(evens, nonprimes));
    dumpset("Odd prime numbers:", set_difference(odds, nonprimes));

    /* Cleanup */
    set_destroy(all);
    set_destroy(evens);
    set_destroy(odds);
    set_destroy(nonprimes);
    set_destroy(primes);

    for (i = 0; i <= n; i++) {
	    free(numbers[i]);
    }
    free(numbers);
}

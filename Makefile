LIST_SRC=linkedlist.c
SET_SRC=set.c
SPAMFILTER_SRC=spamfilter.c common.c $(LIST_SRC) $(SET_SRC)
NUMBERS_SRC=numbers.c common.c $(LIST_SRC) $(SET_SRC)
HEADERS=common.h list.h set.h

all: spamfilter numbers

spamfilter: $(SPAMFILTER_SRC) $(HEADERS) Makefile
	gcc -o $@ $(SPAMFILTER_SRC)

numbers: $(NUMBERS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(NUMBERS_SRC)

clean:
	rm -f *~ *.o *.exe spamfilter numbers

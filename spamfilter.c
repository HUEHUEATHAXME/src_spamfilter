#include "list.h"
#include "set.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"

/*
 * Case-insensitive comparison function for strings.
 */
static int compare_words(void *a, void *b)
{
    return strcasecmp(a, b);
}

/*
 * Returns the set of (unique) words found in the given file.
 */
static set_t *tokenize(char *filename)
{
	set_t *wordset = set_create(compare_words);
	list_t *wordlist = list_create(compare_words);
	list_iter_t *it;
	FILE *f;
	
	f = fopen(filename, "r");
	if (f == NULL) {
		perror("fopen");
		fatal_error("fopen() failed");
	}
	tokenize_file(f, wordlist);
	
	it = list_createiter(wordlist);
	while (list_hasnext(it)) {
                void *tmp = list_next(it);
		set_add(wordset, tmp, tmp);		
	}
	list_destroyiter(it);
	list_destroy(wordlist);
	return wordset;
}

/*
 * Prints a set of words.
 */
static void printwords(char *prefix, set_t *words)
{
	set_iter_t *it;
	
	it = set_createiter(words);
	printf(prefix);
	while (set_hasnext(it)) {
		printf(" %s", set_next(it));
	}
	printf("\n");
	set_destroyiter(it);
}
//print a list of words
static void list_printwords(char *prefix, list_t *words)
{
	list_iter_t *it;
	
	it = list_createiter(words);
	printf(prefix);
	while (list_hasnext(it)) {
		printf(" %s", list_next(it));
	}
	printf("\n");
	list_destroyiter(it);
}
typedef set_t*(*operation_t)(set_t *, set_t *); //A set operation prototype

/*
 * This function does the set operation given as input
 */
set_t *operation_handler(char *dirname, operation_t operation)
{
        list_t *files = find_files(dirname);    //The files within the folder given as input
        list_iter_t *iter = list_createiter(files); //Iterator to iterate over the number of files

        set_t *set, *tmp, *tmp2;
        if (list_hasnext(iter))
            set = tokenize(list_next(iter));

        while (list_hasnext(iter))   {
            tmp = tokenize(list_next(iter));
            tmp2 = set;

            set = operation(set, tmp);  //Do the operation given as input

            set_destroy(tmp);
            set_destroy(tmp2);
        }
        list_destroyiter(iter);
        list_destroy(files);
        return set;

}




/*
 * Main entry point.
 */
int main(int argc, char **argv)
{
	char *spamdir, *nonspamdir, *maildir;
        
	
	if (argc != 4) {
		fprintf(stderr, "usage: %s <spamdir> <nonspamdir> <maildir>\n",
				argv[0]);
		return 1;
	}
	spamdir = argv[1];
	nonspamdir = argv[2];
	maildir = argv[3];
        
        //Find intersection of the spamset. Then the unionset of the non-spam mails
        set_t *spamset = operation_handler(spamdir, set_intersection);
        set_t * nonspamset = operation_handler(nonspamdir, set_union);

        //Create and find the differance set
        set_t *diffset = set_create(compare_words);
        diffset = set_difference(spamset, nonspamset);
        set_destroy(spamset);
        set_destroy(nonspamset);        


        list_t *maillist = find_files(maildir);
        list_iter_t *mailiter = list_createiter(maillist);
        set_t *resultset, *mailset;
        
        while (list_hasnext(mailiter))  {
            mailset = tokenize(list_peeknext(mailiter));    //Peek at the next item within the mailset
            resultset = set_intersection(diffset, mailset);
            printf("%s: %d spam word(s) -> %s\n", list_next(mailiter), set_size(resultset), (set_size(resultset) > 0) ? "SPAM" : "Not spam");   //Print whether the mail is spam or not
            set_destroy(mailset);
            set_destroy(resultset);
        }
        list_destroy(maillist);
        list_destroyiter(mailiter);
        
    return 0;
}

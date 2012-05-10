#include "common.h"
#include "list.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void fatal_error(char *msg)
{
    fprintf(stderr, "fatal error: %s\n", msg);
    exit(1);
}

void tokenize_file(FILE *file, list_t *list)
{
    char *word;
    char buf[101];
    buf[100] = 0;
    
    while (!feof(file)) {
		/* Skip non-letters */
		fscanf(file, "%*[^a-zA-Z0-9'_]");
		/* Scan up to 100 letters */
		if (fscanf(file, "%100[a-zA-Z0-9'_]", buf) == 1) {
	    	word = strdup(buf);
	    	if (word == NULL)
				fatal_error("out of memory");
	    	list_addlast(list, word);
		}
    }
}

struct list *find_files(char *root)
{
    list_t *files;
    char *command;
    FILE *f;
    
    files = list_create(compare_strings);
    command = malloc(strlen(root) + 40);
    if (command == NULL)
        fatal_error("out of memory");
    sprintf(command, "/usr/bin/find %s -not -type d", root);
    f = popen(command, "r");
    if (f == NULL) {
        perror("popen");
        fatal_error("popen() failed");
    }
    while(!feof(f)) {
        char line[300], *path, *p;
        
        /* Read one line of output from the find command */
        if (fgets(line, sizeof(line), f) == NULL)
            break;
        
        /* Strip trailing whitespace */
        p = line+strlen(line)-1;
        while (p >= line && isspace(*p))
            *p-- = 0;
            
        /* Copy and add the path to the list */
        path = strdup(line);
        if (path == NULL)
            fatal_error("out of memory");
        list_addlast(files, path);
    }
    if (fclose(f) < 0) {
        perror("fclose");
        fatal_error("fclose() failed");
    }
    free(command);    
    return files;
}

int compare_strings(void *a, void *b)
{
    return strcmp(a, b);
}

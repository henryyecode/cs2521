// crawl.c ... build a graph of part of the web
// Written by John Shepherd, September 2015
// Uses the cURL library and functions by Vincent Sanders
// <vince@kyllikki.org>

#include <ctype.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "graph.h"
#include "html.h"
#include "set.h"
#include "stack.h"
#include "url_file.h"
#include "queue.h"

#define BUFSIZE 1024

static void setFirstURL (char *, char *);

int main (int argc, char **argv)
{
	URL_FILE *handle;
	char buffer[BUFSIZE];
	char baseURL[BUFSIZE];
	char firstURL[BUFSIZE];
	char next[BUFSIZE];
	int maxURLs;

	if (argc > 2) {
		strcpy (baseURL, argv[1]);
		setFirstURL (baseURL, firstURL);
		maxURLs = atoi (argv[2]);
		if (maxURLs < 40)
			maxURLs = 40;
	} else {
		fprintf (stderr, "Usage: %s BaseURL MaxURLs\n", argv[0]);
		exit (1);
	}
    
    Queue q = newQueue();
	enterQueue(q, firstURL);
	
	Graph g = newGraph(maxURLs);
	
	Set s = newSet();
	
	while (!emptyQueue(q) && nVertices(g) < maxURLs) {
	    char *c = leaveQueue(q);
	    strcpy(next, c);
	    
	    if(strstr(c, "unsw.edu.au") == NULL) {
	        continue;
	    }
	
	    if (!(handle = url_fopen (next, "r"))) {
		    fprintf (stderr, "Couldn't open %s\n", next);
		    exit (1);
	    }
	    
	    while (!url_feof (handle)) {
		    url_fgets (buffer, sizeof (buffer), handle);
		    // fputs(buffer,stdout);
		    int pos = 0;
		    char result[BUFSIZE];
		    memset (result, 0, BUFSIZE);
		    while ((pos = GetNextURL (buffer, next, result, pos)) > 0) {
			    printf ("Found: '%s'\n", result);
			    
			    
			    if (nVertices(g) < maxURLs || (isElem(s,next) && isElem(s,result))) {
			        addEdge(g, next, result);
			    }
			    if (!isElem(s, next)) {
			        insertInto(s,next);	    
	    		    enterQueue(q, result);    
			    }
			    memset (result, 0, BUFSIZE);
		    }
	    }
	    free(c);
	    url_fclose (handle);
	    sleep (1);

    }
    
    dropQueue(q);
    dropGraph(g);
    dropSet(s);
    return 0;
}

// setFirstURL(Base,First)
// - sets a "normalised" version of Base as First
// - modifies Base to a "normalised" version of itself
static void setFirstURL (char *base, char *first)
{
	char *c;
	if ((c = strstr (base, "/index.html")) != NULL) {
		strcpy (first, base);
		*c = '\0';
	} else if (base[strlen (base) - 1] == '/') {
		strcpy (first, base);
		strcat (first, "index.html");
		base[strlen (base) - 1] = '\0';
	} else {
		strcpy (first, base);
		strcat (first, "/index.html");
	}
}


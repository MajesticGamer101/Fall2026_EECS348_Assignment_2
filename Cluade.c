/*
 * Email Priority Queue for a busy CEO, implemented with a custom
 * list-based (dynamic array) MaxHeap built entirely from scratch in
 * plain C (no library heap/priority_queue routines used).
 *
 * Priority order (highest read first):
 *   Boss > Subordinate > Peer > ImportantPerson > OtherPerson
 * Ties within the same category are broken by date: newest first.
 *
 * Commands (one per line, read from a file given as argv[1], or
 * from stdin if no file is given):
 *   EMAIL <sender category>,<subject line>,<date MM-DD-YYYY>
 *   NEXT
 *   READ
 *   COUNT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE     512
#define MAX_CATEGORY 32
#define MAX_SUBJECT  256
#define MAX_DATE     16

/* ---------------------------------------------------------------
 * Email record
 * --------------------------------------------------------------- */
typedef struct {
    char category[MAX_CATEGORY];
    char subject[MAX_SUBJECT];
    char date[MAX_DATE];   /* stored as MM-DD-YYYY for display */
    long dateValue;         /* numeric YYYYMMDD form for comparisons */
} Email;

/* ---------------------------------------------------------------
 * MaxHeap - list-based (dynamic array) implementation
 * --------------------------------------------------------------- */
typedef struct {
    Email *data;
    int size;
    int capacity;
} MaxHeap;

/* Returns a numeric rank for a category name. Higher = higher priority. */
int categoryRank(const char *category) {
    if (strcmp(category, "Boss") == 0)            return 5;
    if (strcmp(category, "Subordinate") == 0)     return 4;
    if (strcmp(category, "Peer") == 0)            return 3;
    if (strcmp(category, "ImportantPerson") == 0) return 2;
    if (strcmp(category, "OtherPerson") == 0)     return 1;
    return 0; /* unknown category - lowest priority */
}

/* Converts "MM-DD-YYYY" into an integer YYYYMMDD so a larger value
 * always represents a later (newer) date. */
long dateToValue(const char *date) {
    int mm = 0, dd = 0, yyyy = 0;
    sscanf(date, "%d-%d-%d", &mm, &dd, &yyyy);
    return (long)yyyy * 10000L + (long)mm * 100L + (long)dd;
}

/* Returns 1 if email a should be considered HIGHER priority than
 * email b (i.e. a belongs closer to the root / front of the queue). */
int higherPriority(const Email *a, const Email *b) {
    int rankA = categoryRank(a->category);
    int rankB = categoryRank(b->category);
    if (rankA != rankB) {
        return rankA > rankB;
    }
    /* Same category: newer date wins (read first) */
    return a->dateValue > b->dateValue;
}

void heapInit(MaxHeap *heap) {
    heap->capacity = 8;
    heap->size = 0;
    heap->data = (Email *)malloc(sizeof(Email) * heap->capacity);
}

void heapFree(MaxHeap *heap) {
    free(heap->data);
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

void heapGrowIfNeeded(MaxHeap *heap) {
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;
        heap->data = (Email *)realloc(heap->data, sizeof(Email) * heap->capacity);
    }
}

void heapSwap(MaxHeap *heap, int i, int j) {
    Email temp = heap->data[i];
    heap->data[i] = heap->data[j];
    heap->data[j] = temp;
}

int heapParent(int i) { return (i - 1) / 2; }
int heapLeft(int i)   { return 2 * i + 1; }
int heapRight(int i)  { return 2 * i + 2; }

/* Moves the element at index i up until the heap property holds */
void siftUp(MaxHeap *heap, int i) {
    while (i > 0 && higherPriority(&heap->data[i], &heap->data[heapParent(i)])) {
        heapSwap(heap, i, heapParent(i));
        i = heapParent(i);
    }
}

/* Moves the element at index i down until the heap property holds */
void siftDown(MaxHeap *heap, int i) {
    int size = heap->size;
    while (1) {
        int left = heapLeft(i);
        int right = heapRight(i);
        int best = i;

        if (left < size && higherPriority(&heap->data[left], &heap->data[best])) {
            best = left;
        }
        if (right < size && higherPriority(&heap->data[right], &heap->data[best])) {
            best = right;
        }
        if (best == i) {
            break;
        }
        heapSwap(heap, i, best);
        i = best;
    }
}

void heapInsert(MaxHeap *heap, Email e) {
    heapGrowIfNeeded(heap);
    heap->data[heap->size] = e;
    siftUp(heap, heap->size);
    heap->size++;
}

/* Returns a pointer to the highest priority email without removing
 * it, or NULL if the heap is empty. */
Email *heapPeekMax(MaxHeap *heap) {
    if (heap->size == 0) return NULL;
    return &heap->data[0];
}

/* Removes the highest priority email from the heap. Does nothing if
 * the heap is empty. */
void heapExtractMax(MaxHeap *heap) {
    if (heap->size == 0) return;
    heap->size--;
    heap->data[0] = heap->data[heap->size];
    if (heap->size > 0) {
        siftDown(heap, 0);
    }
}

/* ---------------------------------------------------------------
 * String helpers
 * --------------------------------------------------------------- */

/* Strips trailing '\n' / '\r' characters left by fgets */
void stripNewline(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

/* Trims leading/trailing spaces in-place */
void trim(char *s) {
    int start = 0;
    int end = (int)strlen(s) - 1;
    while (isspace((unsigned char)s[start])) start++;
    while (end >= start && isspace((unsigned char)s[end])) end--;
    int len = end - start + 1;
    if (len > 0) {
        memmove(s, s + start, len);
    }
    s[len > 0 ? len : 0] = '\0';
}

/* ---------------------------------------------------------------
 * Command processing
 * --------------------------------------------------------------- */
void processLine(char *line, MaxHeap *heap) {
    stripNewline(line);
    trim(line);
    if (strlen(line) == 0) return;

    if (strncmp(line, "EMAIL", 5) == 0) {
        /* Everything after "EMAIL " is "<category>,<subject>,<date>" */
        char *rest = line + 5;
        while (*rest == ' ') rest++;

        char *firstComma = strchr(rest, ',');
        char *lastComma = strrchr(rest, ',');
        if (firstComma == NULL || lastComma == NULL || firstComma == lastComma) {
            return; /* malformed line - ignore */
        }

        Email e;
        memset(&e, 0, sizeof(Email));

        /* category: from rest up to firstComma */
        int catLen = (int)(firstComma - rest);
        if (catLen >= MAX_CATEGORY) catLen = MAX_CATEGORY - 1;
        strncpy(e.category, rest, catLen);
        e.category[catLen] = '\0';
        trim(e.category);

        /* subject: between firstComma+1 and lastComma */
        int subLen = (int)(lastComma - (firstComma + 1));
        if (subLen < 0) subLen = 0;
        if (subLen >= MAX_SUBJECT) subLen = MAX_SUBJECT - 1;
        strncpy(e.subject, firstComma + 1, subLen);
        e.subject[subLen] = '\0';
        trim(e.subject);

        /* date: after lastComma */
        strncpy(e.date, lastComma + 1, MAX_DATE - 1);
        e.date[MAX_DATE - 1] = '\0';
        trim(e.date);

        e.dateValue = dateToValue(e.date);

        heapInsert(heap, e);
    }
    else if (strcmp(line, "NEXT") == 0) {
        Email *top = heapPeekMax(heap);
        if (top == NULL) {
            printf("No emails to read.\n");
        } else {
            printf("Next email:\n");
            printf("Sender: %s\n", top->category);
            printf("Subject: %s\n", top->subject);
            printf("Date: %s\n", top->date);
        }
    }
    else if (strcmp(line, "READ") == 0) {
        /* Removes the highest priority email without displaying it.
         * If the heap is empty, nothing happens. */
        heapExtractMax(heap);
    }
    else if (strcmp(line, "COUNT") == 0) {
        printf("There are %d emails to read.\n", heap->size);
    }
    /* Unknown commands are silently ignored */
}

/* ---------------------------------------------------------------
 * Main driver
 * --------------------------------------------------------------- */
int main(int argc, char *argv[]) {
    MaxHeap heap;
    heapInit(&heap);

    char line[MAX_LINE];
    FILE *input = stdin;

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Could not open file: %s\n", argv[1]);
            heapFree(&heap);
            return 1;
        }
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        processLine(line, &heap);
    }

    if (input != stdin) {
        fclose(input);
    }

    heapFree(&heap);
    return 0;
}
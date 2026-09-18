#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EMAILS 1000
#define MAX_LINE 500

// Represents one email
typedef struct {
    char sender[50];
    char subject[300];
    char date[11];
} Email;

// MaxHeap structure
typedef struct {
    Email emails[MAX_EMAILS];
    int size;
} MaxHeap;


// --------------------------------------------------
// Return priority based on sender category
// Higher number = higher priority
// --------------------------------------------------
int senderPriority(char sender[]) {

    if (strcmp(sender, "Boss") == 0) {
        return 5;
    }
    else if (strcmp(sender, "Subordinate") == 0) {
        return 4;
    }
    else if (strcmp(sender, "Peer") == 0) {
        return 3;
    }
    else if (strcmp(sender, "ImportantPerson") == 0) {
        return 2;
    }
    else {
        return 1;   // OtherPerson
    }
}


// --------------------------------------------------
// Convert MM-DD-YYYY into a number YYYYMMDD
//
// Example:
// 01-03-2025 -> 20250103
// 12-20-2024 -> 20241220
//
// This makes newer dates have larger values.
// --------------------------------------------------
int dateValue(char date[]) {

    int month;
    int day;
    int year;

    sscanf(date, "%d-%d-%d", &month, &day, &year);

    return year * 10000 + month * 100 + day;
}


// --------------------------------------------------
// Determine if email a has higher priority than b
// --------------------------------------------------
int higherPriority(Email a, Email b) {

    int priorityA = senderPriority(a.sender);
    int priorityB = senderPriority(b.sender);

    // First compare sender categories
    if (priorityA > priorityB) {
        return 1;
    }

    if (priorityA < priorityB) {
        return 0;
    }

    // Same sender category, so compare dates
    return dateValue(a.date) > dateValue(b.date);
}


// --------------------------------------------------
// Swap two emails
// --------------------------------------------------
void swapEmails(Email *a, Email *b) {

    Email temp = *a;
    *a = *b;
    *b = temp;
}


// --------------------------------------------------
// Move newly added email upward
// --------------------------------------------------
void upHeap(MaxHeap *heap, int index) {

    while (index > 0) {

        int parent = (index - 1) / 2;

        if (higherPriority(heap->emails[index],
                           heap->emails[parent])) {

            swapEmails(&heap->emails[index],
                       &heap->emails[parent]);

            index = parent;
        }
        else {
            break;
        }
    }
}


// --------------------------------------------------
// Add email to MaxHeap
// --------------------------------------------------
void insertEmail(MaxHeap *heap, Email email) {

    if (heap->size >= MAX_EMAILS) {
        return;
    }

    heap->emails[heap->size] = email;

    upHeap(heap, heap->size);

    heap->size++;
}


// --------------------------------------------------
// Move root downward after removing an email
// --------------------------------------------------
void downHeap(MaxHeap *heap, int index) {

    while (1) {

        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        // Check left child
        if (left < heap->size &&
            higherPriority(heap->emails[left],
                           heap->emails[largest])) {

            largest = left;
        }

        // Check right child
        if (right < heap->size &&
            higherPriority(heap->emails[right],
                           heap->emails[largest])) {

            largest = right;
        }

        // If parent is already largest, stop
        if (largest == index) {
            break;
        }

        swapEmails(&heap->emails[index],
                   &heap->emails[largest]);

        index = largest;
    }
}


// --------------------------------------------------
// READ
//
// Remove highest-priority email without displaying it
// --------------------------------------------------
void readEmail(MaxHeap *heap) {

    // Nothing to read
    if (heap->size == 0) {
        return;
    }

    // Put last email at root
    heap->emails[0] = heap->emails[heap->size - 1];

    heap->size--;

    // Restore MaxHeap
    if (heap->size > 0) {
        downHeap(heap, 0);
    }
}


// --------------------------------------------------
// NEXT
//
// Display highest-priority email without removing it
// --------------------------------------------------
void nextEmail(MaxHeap *heap) {

    if (heap->size == 0) {
        return;
    }

    Email email = heap->emails[0];

    printf("Next email:\n");
    printf("Sender: %s\n", email.sender);
    printf("Subject: %s\n", email.subject);
    printf("Date: %s\n", email.date);
}


// --------------------------------------------------
// COUNT
// --------------------------------------------------
void countEmails(MaxHeap *heap) {

    printf("There are %d emails to read.\n", heap->size);
}


// --------------------------------------------------
// Remove newline from end of string
// --------------------------------------------------
void removeNewline(char string[]) {

    string[strcspn(string, "\r\n")] = '\0';
}


// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main(int argc, char *argv[]) {

    // Make sure filename was provided
    if (argc < 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        printf("Could not open file.\n");
        return 1;
    }

    MaxHeap inbox;
    inbox.size = 0;

    char line[MAX_LINE];

    // Read file one line at a time
    while (fgets(line, sizeof(line), file) != NULL) {

        removeNewline(line);

        // ------------------------------------------
        // EMAIL command
        // ------------------------------------------
        if (strncmp(line, "EMAIL ", 6) == 0) {

            Email email;

            // Skip "EMAIL "
            char *information = line + 6;

            // First comma separates sender
            char *sender = strtok(information, ",");

            // Second field is subject
            char *subject = strtok(NULL, ",");

            // Third field is date
            char *date = strtok(NULL, ",");

            if (sender != NULL &&
                subject != NULL &&
                date != NULL) {

                strcpy(email.sender, sender);
                strcpy(email.subject, subject);
                strcpy(email.date, date);

                insertEmail(&inbox, email);
            }
        }

        // ------------------------------------------
        // NEXT command
        // ------------------------------------------
        else if (strcmp(line, "NEXT") == 0) {

            nextEmail(&inbox);
        }

        // ------------------------------------------
        // READ command
        // ------------------------------------------
        else if (strcmp(line, "READ") == 0) {

            readEmail(&inbox);
        }

        // ------------------------------------------
        // COUNT command
        // ------------------------------------------
        else if (strcmp(line, "COUNT") == 0) {

            countEmails(&inbox);
        }
    }

    fclose(file);

    return 0;
}
/*
 Alex Asch, alasch
 CS101 PA3
 List.c
 contains the implementation of list adt
 */
#include "List.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct NodeObj *Node;

typedef struct NodeObj {
    char *key;
    char *value;
    Node prev;
    Node next;
} NodeObj;

typedef struct ListObj {
    Node front;
    Node back;
    int length;
    int index;
    Node cursor;
} ListObj;

Node newNode(char *key, char *value) {
    Node N = calloc(1, sizeof(NodeObj));
    N->key = strndup(key, 2048);
    N->value = strndup(value, 2048);
    N->next = NULL;
    N->prev = NULL;
    return (N);
}

void freeNode(Node *pN) {
    if (pN != NULL && *pN != NULL) {
        free((*pN)->key);
        free((*pN)->value);
        free(*pN);
        *pN = NULL;
    }
}

List newList() {
    List L;
    L = calloc(1, sizeof(ListObj));
    L->front = L->back = NULL;
    L->length = 0;
    L->cursor = NULL;
    return L;
}
// Frees the list pointed to by pointer pL
void freeList(List *pL) {
    if (pL != NULL && *pL != NULL) {
        while ((length(*pL) > 0)) {
            deleteFront(*pL);
        }
        free(*pL);
        *pL = NULL;
    }
}
// Retruns the length of list L,
// PREQ: Non null List reference
int length(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling length on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    return L->length;
}

// Returns the current index of the cursor, or -1 if there is no cursor.
// PREQ: Non Null List
int index(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling index on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->cursor) {

        return L->index;
    }
    return -1;
}

// Returns the front of List L
// Requires a non null and non empty list
char *front(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling front on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        fprintf(stderr, "List error, calling front on an empty list\n");
        exit(EXIT_FAILURE);
    }
    return L->front->key;
}

// Returns the back of List L
// requires a non null non empty list
char *back(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling back on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        fprintf(stderr, "List error, calling back on an empty list\n");
        exit(EXIT_FAILURE);
    }
    return L->back->key;
}

// Returns the cursor element of L
// Requires a non null list, and a defined cursor
char *getkey(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling get on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (!(index(L) >= 0)) {
        fprintf(stderr, "List Error, calling get on a NULL cursor\n");
        exit(EXIT_FAILURE);
    }
    return L->cursor->key;
}
char *getval(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling get on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (!(index(L) >= 0)) {
        fprintf(stderr, "List Error, calling get on a NULL cursor\n");
        exit(EXIT_FAILURE);
    }
    return L->cursor->value;
}

// Clears the List and the memory used by the nodes
// PREQ: The List is not NULL
void clear(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling get on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L != NULL) {
        while ((length(L) > 0)) {
            deleteFront(L);
        }
    }

    L->cursor = NULL;
}

// Deletes the front of List L
// PREQ: A non null non empty list
void deleteFront(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling delete front on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        fprintf(stderr, "List Error, calling length on an empty list reference\n");
        exit(EXIT_FAILURE);
    }
    Node tofree = L->front;
    if (L->length == 1) {
        L->front = L->back = NULL;
    } else {
        L->front = L->front->next;
    }
    if (L->cursor) {
        L->index--;
    }
    L->length--;
    freeNode(&tofree);
}

// Deletes the back of List L
// PREQ: A non null non empty list
void deleteBack(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling delete back on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        fprintf(stderr, "List Error, calling delete back on an empty list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->cursor == L->back) {
        L->index = -1;
    }
    if (L->length == 1) {
        freeNode(&(L->back));
    } else {
        L->back = L->back->prev;
        freeNode(&(L->back->next));
    }
    L->length--;
}

// ADds the given index integer x to the beginning of list L
// Requires a non null list
void prepend(List L, char *key, char *val) {
    if (!L) {
        fprintf(stderr, "List Error, prepend on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    Node toadd = newNode(key, val);
    if (L->length == 0) {
        L->front = L->back = toadd;
    } else {
        toadd->next = L->front;
        L->front->prev = toadd;
        L->front = toadd;
    }
    if (L->cursor) {
        L->index++;
    }
    L->length++;
}

// Add the given integer to the back of List L
// Requires a non null list
void append(List L, char *key, char *val) {
    if (!L) {
        fprintf(stderr, "List Error, calling append on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    Node toadd = newNode(key, val);
    if (L->length == 0) {
        L->front = L->back = toadd;
    } else {
        toadd->prev = L->back;
        L->back->next = toadd;
        L->back = toadd;
    }
    L->length++;
}

// Moves the cursor to the front of the list
// Requires a non null non empty list
void moveFront(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling move front on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        return;
    }
    L->index = 0;
    L->cursor = L->front;
}
// Moves the cursor to the front of the lsit
// Requires a non null non empty list

void moveBack(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling move back on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        return;
    }
    L->index = L->length - 1;
    L->cursor = L->back;
}

// Moves the cursor to the previous index, if cursor is undefined do nothing, if
// cursor is off index set it to -1 Requires a non null list
void movePrev(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling delete back on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (!L->cursor) {
        return;
    }
    L->index--;
    L->cursor = L->cursor->prev;
}

// Moves the cursor tot he next index, if the cursor is undefined do nothing, if
// it falls of the back set it to -1 PREQ: non null list
void moveNext(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling move next on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (!L->cursor) {
        return;
    }

    L->index++;
    if (L->index == L->length) {
        L->index = -1;
        L->cursor = NULL;
        return;
    }
    L->cursor = L->cursor->next;
}

// Deletes the node at the cursor
// PREq: non null cursor and non null list.
void delete (List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling delete on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    if (L->cursor == NULL) {
        fprintf(stderr, "List Error, calling delete on a NULL cursor\n");
        exit(EXIT_FAILURE);
    }
    if (L->cursor == L->back) {
        deleteBack(L);
        return;
    }
    if (L->cursor == L->front) {
        deleteFront(L);

    } else {
        if (L->cursor->prev) {
            L->cursor->prev->next = L->cursor->next;
        }
        if (L->cursor->next) {
            L->cursor->next->prev = L->cursor->prev;
        }
        freeNode(&L->cursor);
        L->index = -1;
        L->length--;
    }
}

// Prints the list L
// PREQ: non null List
void printList(int towrite, List L) {
    Node N = NULL;

    if (L == NULL) {
        printf("List Error: calling printList() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }
    for (N = L->front; N != NULL; N = N->next) {
        printf(out, "%s, %s\n", N->key, N->val);
    }
    printf(out, "\n");
}
// Copies the List L and returns the copied List
// PREQ: non null list
List copyList(List L) {
    if (!L) {
        fprintf(stderr, "List Error, calling cpoy list on a NULL list reference\n");
        exit(EXIT_FAILURE);
    }
    List newlist = newList();
    Node A = NULL;
    A = L->front;
    while (A != NULL) {
        append(newlist, A->key, A->val);
        A = A->next;
    }
    return newlist;
}

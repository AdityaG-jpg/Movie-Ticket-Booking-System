/*
 * stage2.h - STAGE 2 : Singly linked lists
 *            Movie catalogue + recently viewed history
 */
#ifndef STAGE2_H
#define STAGE2_H

#include "common.h"

struct MovieNode {
    int  id;
    char name[NAME_LEN];
    int  price;
    struct MovieNode* next;
};

struct RecentNode {
    int  id;
    char name[NAME_LEN];
    struct RecentNode* next;
};

struct MovieNode*  movieHead  = NULL;
struct RecentNode* recentHead = NULL;

/* ================================================================
 *                 MOVIE CATALOGUE (SINGLY LINKED LIST)
 * ================================================================ */

/* Adds a new movie to the end of the list. Returns 1 on success */
int addMovie(int id, const char* name, int price) {
    struct MovieNode* newNode;
    struct MovieNode* temp;

    newNode = (struct MovieNode*)malloc(sizeof(struct MovieNode));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return 0;
    }
    newNode->id = id;
    strncpy(newNode->name, name, NAME_LEN - 1);
    newNode->name[NAME_LEN - 1] = '\0';
    newNode->price = price;
    newNode->next = NULL;

    if (movieHead == NULL) {
        movieHead = newNode;
    } else {
        temp = movieHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    return 1;
}

/* Deletes a movie by ID. Returns 1 if deleted, 0 if not found */
int deleteMovie(int id) {
    struct MovieNode* temp = movieHead;
    struct MovieNode* prev = NULL;

    if (movieHead == NULL) {
        printf("No movies available to delete.\n");
        return 0;
    }

    if (temp->id == id) {
        movieHead = temp->next;
        free(temp);
        return 1;
    }

    while (temp != NULL && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Movie ID %d not found.\n", id);
        return 0;
    }
    prev->next = temp->next;
    free(temp);
    return 1;
}

/* Displays all movies in the list */
void viewMovies(void) {
    struct MovieNode* temp = movieHead;

    if (movieHead == NULL) {
        printf("\nNo movies currently available.\n");
        return;
    }
    printf("\n========== AVAILABLE MOVIES ==========\n");
    printf("%-5s %-20s %-10s\n", "ID", "Movie", "Price");
    while (temp != NULL) {
        printf("%-5d %-20s Rs.%d\n", temp->id, temp->name, temp->price);
        temp = temp->next;
    }
}

/* Linear search by ID. Returns node or NULL */
struct MovieNode* searchMovie(int id) {
    struct MovieNode* temp = movieHead;
    while (temp != NULL) {
        if (temp->id == id) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

int countMovies(void) {
    int n = 0;
    struct MovieNode* temp = movieHead;
    while (temp != NULL) {
        n++;
        temp = temp->next;
    }
    return n;
}

void freeMovies(void) {
    struct MovieNode* temp;
    while (movieHead != NULL) {
        temp = movieHead;
        movieHead = movieHead->next;
        free(temp);
    }
}

/* ================================================================
 *               RECENTLY VIEWED (SINGLY LINKED LIST)
 * ================================================================ */

/* Removes a movie from the history (if present) */
void removeRecentlyViewed(int id) {
    struct RecentNode* cur = recentHead;
    struct RecentNode* prev = NULL;

    while (cur != NULL && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) return;

    if (prev == NULL)
        recentHead = cur->next;
    else
        prev->next = cur->next;
    free(cur);
}

/* Inserts at the head so the latest view is first.
 * A movie already in the list is moved to the front instead of
 * being repeated, and only the last RECENT_LIMIT views are kept. */
void addRecentlyViewed(int id, const char* name) {
    struct RecentNode* newNode;
    struct RecentNode* cur;
    struct RecentNode* extra;
    int count;

    removeRecentlyViewed(id);

    newNode = (struct RecentNode*)malloc(sizeof(struct RecentNode));
    if (!newNode) return;
    newNode->id = id;
    strncpy(newNode->name, name, NAME_LEN - 1);
    newNode->name[NAME_LEN - 1] = '\0';
    newNode->next = recentHead;
    recentHead = newNode;

    /* trim the list to RECENT_LIMIT nodes */
    cur = recentHead;
    count = 1;
    while (cur->next != NULL && count < RECENT_LIMIT) {
        cur = cur->next;
        count++;
    }
    while (cur->next != NULL) {
        extra = cur->next;
        cur->next = extra->next;
        free(extra);
    }
}

void displayRecentlyViewed(void) {
    struct RecentNode* temp = recentHead;

    if (recentHead == NULL) {
        printf("\nNo recently viewed movies.\n");
        return;
    }
    printf("\n========== RECENTLY VIEWED ==========\n");
    while (temp != NULL) {
        printf("Movie ID: %d | Name: %s\n", temp->id, temp->name);
        temp = temp->next;
    }
}

void freeRecent(void) {
    struct RecentNode* temp;
    while (recentHead != NULL) {
        temp = recentHead;
        recentHead = recentHead->next;
        free(temp);
    }
}

#endif

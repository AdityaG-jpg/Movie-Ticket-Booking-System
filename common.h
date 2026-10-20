/*
 * common.h - constants, the Booking structure and input helpers
 *            (used by every stage)
 */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SEATS      5      /* seats per booking (size of seat stack)  */
#define MAX_SEAT_NO    60     /* every screen has seats 1..60            */
#define MAX_BOOKINGS   50     /* size of booking queue                   */
#define MAX_THEATRES   10     /* vertices in the theatre graph           */
#define MAX_SHOWING    10     /* movies one theatre can show             */
#define MAX_ROUTE_KM   500    /* largest distance allowed for one route  */
#define MAX_PRICE      5000   /* keeps totals inside a 16-bit int        */
#define RECENT_LIMIT   5      /* recently viewed list keeps last 5       */
#define NAME_LEN       30
#define HASH_SIZE      11     /* buckets in booking hash table (prime)   */
#define INF            30000  /* "infinite" distance, fits a 16-bit int  */

struct Booking {
    int  bookingId;
    int  movieId;
    int  theatreId;
    char movieName[NAME_LEN];
    char theatreName[NAME_LEN];
    int  seats[MAX_SEATS];
    int  seatCount;
    int  totalAmount;
};

/* ================================================================
 *                       UTILITY FUNCTIONS
 * ================================================================ */

/* Clears invalid keyboard input (also stops on EOF) */
void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Reads an int safely. Returns 1 on success, 0 on failure */
int getInt(int* x) {
    int r = scanf("%d", x);
    if (r == EOF) {                 /* input closed: no way to continue */
        printf("\nInput ended. Exiting.\n");
        exit(0);
    }
    if (r != 1) {
        clearInput();
        return 0;
    }
    return 1;
}

/* Reads a name that may contain spaces (max 29 chars) */
int readName(char* buf) {
    if (scanf(" %29[^\n]", buf) != 1) {
        clearInput();
        return 0;
    }
    clearInput();
    return 1;
}

/* Case-insensitive string compare */
int cmpNoCase(const char* a, const char* b) {
    int x, y;
    while (*a && *b) {
        x = tolower((unsigned char)*a);
        y = tolower((unsigned char)*b);
        if (x != y) return x - y;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* Case-insensitive prefix check */
int startsWithNoCase(const char* s, const char* prefix) {
    while (*prefix) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix))
            return 0;
        s++;
        prefix++;
    }
    return 1;
}

#endif

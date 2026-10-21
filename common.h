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
#include <limits.h>
#include <errno.h>

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
static void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Reads an int safely. Returns 1 on success, 0 on failure.
 * The whole word typed must be a number that fits in an int, so
 * "12abc" or "99999999999" are rejected instead of being half-read
 * or silently wrapped around. */
static int getInt(int* x) {
    char word[40];
    char* end;
    long v;
    int r;

    r = scanf("%39s", word);
    if (r == EOF) {                 /* input closed: no way to continue */
        printf("\nInput ended. Exiting.\n");
        exit(0);
    }
    errno = 0;
    v = strtol(word, &end, 10);
    if (*end != '\0' || errno == ERANGE || v > INT_MAX || v < INT_MIN) {
        clearInput();               /* drop the rest of the bad line */
        return 0;
    }
    *x = (int)v;
    return 1;
}

/* Reads a name that may contain spaces (max 29 chars).
 * Longer text is cut to 29 chars and trailing spaces are removed.
 * Returns 1 on success, 0 if nothing usable was read. */
static int readName(char* buf) {
    int len;

    if (scanf(" %29[^\n]", buf) != 1) {
        clearInput();
        return 0;
    }
    clearInput();
    len = (int)strlen(buf);
    while (len > 0 && isspace((unsigned char)buf[len - 1]))
        buf[--len] = '\0';
    return len > 0;
}

/* Case-insensitive string compare */
static int cmpNoCase(const char* a, const char* b) {
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
static int startsWithNoCase(const char* s, const char* prefix) {
    while (*prefix) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix))
            return 0;
        s++;
        prefix++;
    }
    return 1;
}

#endif

/*
 * stage1.h - STAGE 1 : Linear data structures (array based)
 *            Stack : seat selection with undo (LIFO)
 *            Queue : confirmed bookings in order (FIFO)
 */
#ifndef STAGE1_H
#define STAGE1_H

#include "common.h"

/* ================================================================
 *                  STACK : SEAT SELECTION / UNDO
 * ================================================================ */
struct SeatStack {
    int seats[MAX_SEATS];
    int top;
};

struct SeatStack seatStack;

/* Stack initialization */
void initializeStack(void) {
    seatStack.top = -1;
}

/* Stack PUSH operation. Returns 1 on success, 0 if full */
int pushSeat(int seat) {
    if (seatStack.top == MAX_SEATS - 1) {
        printf("Maximum seats reached.\n");
        return 0;
    }
    seatStack.seats[++seatStack.top] = seat;
    return 1;
}

/* Stack POP operation. Returns the seat, or -1 if empty */
int popSeat(void) {
    if (seatStack.top == -1) {
        printf("No seat available to undo.\n");
        return -1;
    }
    return seatStack.seats[seatStack.top--];
}

int isStackEmpty(void) {
    return seatStack.top == -1;
}

int stackSize(void) {
    return seatStack.top + 1;
}

/* Checks if a seat is already selected (prevents picking it twice) */
int stackContains(int seat) {
    int i;
    for (i = 0; i <= seatStack.top; i++) {
        if (seatStack.seats[i] == seat) return 1;
    }
    return 0;
}

/* Displays selected seats */
void displaySeats(void) {
    int i;

    if (seatStack.top == -1) {
        printf("No seats selected.\n");
        return;
    }

    printf("Selected Seats: ");
    for (i = 0; i <= seatStack.top; i++) {
        printf("%d ", seatStack.seats[i]);
    }
    printf("\n");
}

/* ================================================================
 *                  QUEUE : CONFIRMED BOOKINGS
 * ================================================================ */
struct BookingQueue {
    struct Booking bookings[MAX_BOOKINGS];
    int rear;
};

struct BookingQueue bookingQueue;

/* Queue initialization */
void initializeQueue(void) {
    bookingQueue.rear = -1;
}

int isQueueFull(void) {
    return bookingQueue.rear == MAX_BOOKINGS - 1;
}

/* Queue ENQUEUE operation.
 * Returns a pointer to the stored booking (used by the hash table),
 * or NULL if the queue is full. */
struct Booking* enqueueBooking(struct Booking booking) {
    if (isQueueFull()) {
        printf("Booking queue is full.\n");
        return NULL;
    }
    bookingQueue.bookings[++bookingQueue.rear] = booking;
    return &bookingQueue.bookings[bookingQueue.rear];
}

/* Prints one booking */
void printBooking(const struct Booking* b) {
    int j;

    printf("Booking ID: %d\n", b->bookingId);
    printf("Movie: %s\n", b->movieName);
    printf("Theatre: %s\n", b->theatreName);
    printf("Seats: ");
    for (j = 0; j < b->seatCount; j++) {
        printf("%d ", b->seats[j]);
    }
    printf("\nTotal Amount: Rs.%d\n", b->totalAmount);
}

/* Displays bookings stored in Queue (front to rear) */
void viewBookings(void) {
    int i;

    if (bookingQueue.rear == -1) {
        printf("\nNo bookings available.\n");
        return;
    }

    printf("\n========== BOOKING QUEUE ==========\n");
    for (i = 0; i <= bookingQueue.rear; i++) {
        printf("\n");
        printBooking(&bookingQueue.bookings[i]);
        printf("------------------------------------\n");
    }
}

/* Linear search through all bookings: is this seat already taken
 * for the same movie at the same theatre? */
int isSeatBooked(int movieId, int theatreId, int seat) {
    int i, j;
    struct Booking* b;

    for (i = 0; i <= bookingQueue.rear; i++) {
        b = &bookingQueue.bookings[i];
        if (b->movieId != movieId || b->theatreId != theatreId) continue;
        for (j = 0; j < b->seatCount; j++) {
            if (b->seats[j] == seat) return 1;
        }
    }
    return 0;
}

#endif

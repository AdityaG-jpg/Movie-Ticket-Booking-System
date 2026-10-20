/*
 * SMART MOVIE TICKET BOOKING SYSTEM  -  SLA STAGE 3
 * Group 13 : Rishi Ghoricha (13), Aditya Ghosh (14)
 * ---------------------------------------------------------------
 * common.h : constants, Booking structure, input helpers
 * stage1.h : Stack (seat select / undo), Queue (confirmed bookings)
 * stage2.h : Singly Linked Lists (movie catalogue, recently viewed)
 * stage3.h : BST (movie index), Graph (theatre network: BFS, DFS,
 *            Dijkstra), Hashing (booking lookup), Searching
 *            (linear vs binary)
 * main.c   : booking workflow, admin and user menus, main()
 *
 * Compile (gcc) :  gcc main.c -o movie_booking
 * Turbo C       :  open MAIN.C and press Ctrl+F9
 */
#include "common.h"
#include "stage1.h"
#include "stage2.h"
#include "stage3.h"

/* ================================================================
 *     CATALOGUE : keeps list, BST, theatres and history in sync
 * ================================================================ */

/* Validates and adds a movie to the linked list AND the BST index.
 * Returns 1 on success, 0 on error (message already printed). */
int catalogAddMovie(int id, const char* name, int price) {
    if (id <= 0) {
        printf("Movie ID must be a positive number.\n");
        return 0;
    }
    if (price <= 0 || price > MAX_PRICE) {
        printf("Price must be between 1 and %d.\n", MAX_PRICE);
        return 0;
    }
    if (strlen(name) == 0) {
        printf("Movie name cannot be empty.\n");
        return 0;
    }
    if (searchMovie(id) != NULL) {
        printf("Movie ID %d already exists.\n", id);
        return 0;
    }
    if (bstSearchByName(name) != NULL) {
        printf("A movie named \"%s\" already exists.\n", name);
        return 0;
    }
    if (!addMovie(id, name, price)) return 0;
    bstInsert(id, name);
    return 1;
}

/* Removes a movie from the linked list, BST, every theatre and the
 * recently viewed history. Returns 1 on success. */
int catalogDeleteMovie(int id) {
    struct MovieNode* m = searchMovie(id);
    char name[NAME_LEN];

    if (m == NULL) {
        printf("Movie ID %d not found.\n", id);
        return 0;
    }
    strcpy(name, m->name);              /* copy before the node is freed */

    if (!deleteMovie(id)) return 0;
    bstDelete(name);
    removeMovieFromTheatres(id);
    removeRecentlyViewed(id);
    return 1;
}

/* ================================================================
 *                       BOOKING WORKFLOW
 * ----------------------------------------------------------------
 *   movie      -> picked from the linked list
 *   theatre    -> picked from the graph (manually or nearest by Dijkstra)
 *   seats      -> pushed on the stack (undo = pop)
 *   confirmed  -> enqueued in the booking queue
 *   booking ID -> inserted in the hash table for O(1) lookup
 * ================================================================ */
int nextBookingId = 1001;

/* Lets the user choose a theatre showing the movie.
 * Returns the theatre index, or -1 if cancelled. */
int chooseTheatre(int movieId) {
    int choice, t, area;

    if (theatreCount == 0) {
        printf("No theatres available.\n");
        return -1;
    }
    if (listTheatresShowing(movieId) == 0) {
        printf("Sorry, this movie is not showing in any theatre right now.\n");
        return -1;
    }

    printf("\n1. Choose a theatre from the list\n");
    printf("2. Find the nearest theatre to my area\n");
    printf("Enter choice: ");
    if (!getInt(&choice) || (choice != 1 && choice != 2)) {
        printf("Invalid choice.\n");
        return -1;
    }

    if (choice == 2) {
        displayTheatres();
        printf("Enter your area number: ");
        if (!getInt(&area)) {
            printf("Invalid input.\n");
            return -1;
        }
        t = findNearestTheatre(area, movieId);
        if (t == -1) return -1;
        printf("Book at this theatre? (1. Yes / 2. No): ");
        if (!getInt(&choice) || choice != 1) {
            printf("Booking cancelled.\n");
            return -1;
        }
        return t;
    }

    printf("Enter Theatre Number: ");
    if (!getInt(&t)) {
        printf("Invalid input.\n");
        return -1;
    }
    if (!validTheatre(t) || !theatreShowsMovie(t, movieId)) {
        printf("This theatre is not showing the selected movie.\n");
        return -1;
    }
    return t;
}

void bookTicket(void) {
    int movieId, theatreId, numberOfSeats, seat, actionChoice, confirmChoice, removedSeat, i;
    struct MovieNode* selectedMovie;
    struct Booking newBooking;
    struct Booking* stored;

    if (isQueueFull()) {
        printf("Sorry, no more bookings can be accepted.\n");
        return;
    }

    initializeStack();
    viewMovies();
    if (movieHead == NULL) return;

    printf("\nEnter Movie ID: ");
    if (!getInt(&movieId)) {
        printf("Invalid Movie ID.\n");
        return;
    }

    selectedMovie = searchMovie(movieId);
    if (selectedMovie == NULL) {
        printf("Movie not found.\n");
        return;
    }

    addRecentlyViewed(selectedMovie->id, selectedMovie->name);

    printf("\nSelected Movie: %s\n", selectedMovie->name);
    printf("Ticket Price: Rs.%d\n", selectedMovie->price);

    theatreId = chooseTheatre(movieId);
    if (theatreId == -1) return;
    printf("\nTheatre: %s (%s)\n", theatres[theatreId].name, theatres[theatreId].area);

    printf("How many seats? (1-%d): ", MAX_SEATS);
    if (!getInt(&numberOfSeats) || numberOfSeats < 1 || numberOfSeats > MAX_SEATS) {
        printf("Invalid number of seats.\n");
        return;
    }

    i = 0;
    while (i < numberOfSeats) {
        printf("Enter Seat Number %d (1-%d, 0 to cancel): ", i + 1, MAX_SEAT_NO);
        if (!getInt(&seat) || seat < 0 || seat > MAX_SEAT_NO) {
            printf("Invalid seat number.\n");
            continue;
        }
        if (seat == 0) {
            printf("Booking cancelled.\n");
            return;
        }
        if (isSeatBooked(movieId, theatreId, seat)) {
            printf("Seat %d is already booked. Please choose another.\n", seat);
            continue;
        }
        if (stackContains(seat)) {
            printf("You have already selected seat %d.\n", seat);
            continue;
        }
        pushSeat(seat);
        i++;
    }

    while (1) {
        printf("\n");
        displaySeats();
        if (isStackEmpty()) {
            printf("No seats remaining. Booking cancelled.\n");
            return;
        }

        printf("\nOptions:\n");
        printf("1. Undo last selected seat\n");
        printf("2. Proceed to summary & confirmation\n");
        printf("3. Cancel booking\n");
        printf("Enter choice: ");
        if (!getInt(&actionChoice)) {
            printf("Invalid input.\n");
            continue;
        }

        if (actionChoice == 1) {
            removedSeat = popSeat();
            if (removedSeat != -1) printf("Seat %d removed.\n", removedSeat);
        } else if (actionChoice == 2) {
            break;
        } else if (actionChoice == 3) {
            printf("Booking cancelled.\n");
            return;
        } else {
            printf("Invalid choice.\n");
        }
    }

    printf("\n========== BOOKING SUMMARY ==========\n");
    printf("Movie: %s\n", selectedMovie->name);
    printf("Theatre: %s (%s)\n", theatres[theatreId].name, theatres[theatreId].area);
    displaySeats();
    printf("Total: Rs.%d\n", selectedMovie->price * stackSize());

    printf("\nConfirm Booking? (1. Yes / 2. No): ");
    if (!getInt(&confirmChoice) || confirmChoice != 1) {
        printf("Booking cancelled.\n");
        return;
    }

    newBooking.bookingId = nextBookingId;
    newBooking.movieId = movieId;
    newBooking.theatreId = theatreId;
    strcpy(newBooking.movieName, selectedMovie->name);
    strcpy(newBooking.theatreName, theatres[theatreId].name);
    newBooking.seatCount = stackSize();
    newBooking.totalAmount = selectedMovie->price * newBooking.seatCount;
    for (i = 0; i < newBooking.seatCount; i++) {
        newBooking.seats[i] = seatStack.seats[i];
    }

    stored = enqueueBooking(newBooking);
    if (stored == NULL) return;
    hashInsert(stored);
    nextBookingId++;

    printf("\n========== BOOKING SUCCESSFUL ==========\n");
    printf("Booking ID: %d  (keep this to look up your booking)\n", stored->bookingId);
}

/* User feature: look up a booking by its ID using the hash table */
void findMyBooking(void) {
    int id, comparisons;
    struct Booking* b;

    printf("Enter Booking ID: ");
    if (!getInt(&id)) {
        printf("Invalid input.\n");
        return;
    }

    b = hashSearch(id, &comparisons);
    if (b == NULL) {
        printf("No booking found with ID %d.\n", id);
        return;
    }
    printf("\n========== YOUR BOOKING ==========\n");
    printBooking(b);
    printf("(Found in hash bucket %d after %d comparison(s))\n",
           hashFunction(id), comparisons);
}

/* ================================================================
 *                         ADMIN MENU
 * ================================================================ */
void adminAddMovie(void) {
    int id, price;
    char name[NAME_LEN];

    printf("Enter Movie ID: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }
    printf("Enter Movie Name: ");
    if (!readName(name)) { printf("Invalid input.\n"); return; }
    printf("Enter Price: ");
    if (!getInt(&price)) { printf("Invalid input.\n"); return; }

    if (catalogAddMovie(id, name, price))
        printf("Movie added successfully!\n");
}

void adminDeleteMovie(void) {
    int id;

    printf("Enter Movie ID to delete: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }
    if (catalogDeleteMovie(id))
        printf("Movie ID %d deleted successfully.\n", id);
}

void adminCompareSearch(void) {
    int id;

    viewMovies();
    printf("\nEnter Movie ID to search: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }
    compareSearches(id);
}

void adminAddTheatre(void) {
    char name[NAME_LEN], area[NAME_LEN];
    int t;

    printf("Enter Theatre Name: ");
    if (!readName(name)) { printf("Invalid input.\n"); return; }
    printf("Enter Area / Town: ");
    if (!readName(area)) { printf("Invalid input.\n"); return; }
    t = addTheatre(name, area);
    if (t != -1)
        printf("Theatre added as number %d.\n", t);
}

void adminAddRoute(void) {
    int u, v, d;

    displayNetwork();
    if (theatreCount < 2) {
        printf("Add at least two theatres first.\n");
        return;
    }
    printf("Enter first theatre number: ");
    if (!getInt(&u)) { printf("Invalid input.\n"); return; }
    printf("Enter second theatre number: ");
    if (!getInt(&v)) { printf("Invalid input.\n"); return; }
    printf("Enter distance (km): ");
    if (!getInt(&d)) { printf("Invalid input.\n"); return; }

    if (addRoute(u, v, d))
        printf("Route added: %s <-> %s (%d km)\n", theatres[u].area, theatres[v].area, d);
}

void adminAssignMovie(void) {
    int t, id;

    displayTheatres();
    if (theatreCount == 0) return;
    viewMovies();
    printf("\nEnter theatre number: ");
    if (!getInt(&t)) { printf("Invalid input.\n"); return; }
    printf("Enter Movie ID: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }

    if (assignMovieToTheatre(t, id))
        printf("Movie ID %d now showing at %s.\n", id, theatres[t].name);
}

void adminTraversal(int useBfs) {
    int s;

    displayNetwork();
    if (theatreCount == 0) return;
    printf("Enter starting theatre number: ");
    if (!getInt(&s)) { printf("Invalid input.\n"); return; }
    if (useBfs) bfs(s);
    else dfs(s);
}

void adminShortestRoute(void) {
    int s, d;

    displayNetwork();
    if (theatreCount == 0) return;
    printf("Enter source theatre number: ");
    if (!getInt(&s)) { printf("Invalid input.\n"); return; }
    printf("Enter destination theatre number: ");
    if (!getInt(&d)) { printf("Invalid input.\n"); return; }
    shortestRoute(s, d);
}

void theatreMenu(void) {
    int choice;

    while (1) {
        printf("\n====== THEATRE NETWORK (GRAPH) ======\n");
        printf("1. View Theatres\n");
        printf("2. Add Theatre\n");
        printf("3. Add Route Between Theatres\n");
        printf("4. Assign Movie to Theatre\n");
        printf("5. View Network (Adjacency List)\n");
        printf("6. BFS Traversal\n");
        printf("7. DFS Traversal\n");
        printf("8. Shortest Route (Dijkstra)\n");
        printf("9. Back\n");
        printf("Enter choice: ");

        if (!getInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: displayTheatres(); break;
            case 2: adminAddTheatre(); break;
            case 3: adminAddRoute(); break;
            case 4: adminAssignMovie(); break;
            case 5: displayNetwork(); break;
            case 6: adminTraversal(1); break;
            case 7: adminTraversal(0); break;
            case 8: adminShortestRoute(); break;
            case 9: return;
            default: printf("Invalid choice.\n");
        }
    }
}

void adminMenu(void) {
    int choice;

    while (1) {
        printf("\n========== ADMIN MENU ==========\n");
        printf("1. View Movies (Linked List)\n");
        printf("2. Add Movie\n");
        printf("3. Delete Movie\n");
        printf("4. View Bookings (Queue)\n");
        printf("5. Movie Index Info (BST)\n");
        printf("6. Theatre Network (Graph)\n");
        printf("7. Booking Hash Table (Hashing)\n");
        printf("8. Compare Linear vs Binary Search\n");
        printf("9. Back\n");
        printf("Enter choice: ");

        if (!getInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: viewMovies(); break;
            case 2: adminAddMovie(); break;
            case 3: adminDeleteMovie(); break;
            case 4: viewBookings(); break;
            case 5: showBSTInfo(); break;
            case 6: theatreMenu(); break;
            case 7: displayHashTable(); break;
            case 8: adminCompareSearch(); break;
            case 9: return;
            default: printf("Invalid choice.\n");
        }
    }
}

/* ================================================================
 *                          USER MENU
 * ================================================================ */
void searchById(void) {
    int id;
    struct MovieNode* m;

    printf("Enter Movie ID to Search: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }
    m = searchMovie(id);
    if (m != NULL) {
        printf("\n[Found] ID: %d | Movie: %s | Price: Rs.%d\n", m->id, m->name, m->price);
        addRecentlyViewed(m->id, m->name);
    } else {
        printf("Movie not found.\n");
    }
}

/* Exact name search in the BST, falls back to prefix search */
void searchByName(void) {
    char name[NAME_LEN];
    struct BSTNode* node;
    struct MovieNode* m;

    printf("Enter Movie Name (or starting letters): ");
    if (!readName(name)) { printf("Invalid input.\n"); return; }

    node = bstSearchByName(name);
    if (node != NULL) {
        m = searchMovie(node->id);
        if (m) {
            printf("\n[Found] ID: %d | Movie: %s | Price: Rs.%d\n", m->id, m->name, m->price);
            addRecentlyViewed(m->id, m->name);
            return;
        }
    }

    printf("\nNo exact match. Movies starting with \"%s\":\n", name);
    if (bstPrefixSearch(name) == 0)
        printf("  No movie found.\n");
}

void nearestTheatre(void) {
    int id, area;
    struct MovieNode* m;

    viewMovies();
    if (movieHead == NULL) return;
    printf("\nEnter Movie ID: ");
    if (!getInt(&id)) { printf("Invalid input.\n"); return; }
    m = searchMovie(id);
    if (m == NULL) {
        printf("Movie not found.\n");
        return;
    }
    addRecentlyViewed(m->id, m->name);

    displayTheatres();
    if (theatreCount == 0) return;
    printf("Enter your area number: ");
    if (!getInt(&area)) { printf("Invalid input.\n"); return; }
    findNearestTheatre(area, id);
}

void userMenu(void) {
    int choice;

    while (1) {
        printf("\n========== USER MENU ==========\n");
        printf("1. View Movies\n");
        printf("2. View Movies A-Z (BST)\n");
        printf("3. Search Movie by ID\n");
        printf("4. Search Movie by Name (BST)\n");
        printf("5. Find Nearest Theatre for a Movie (Graph)\n");
        printf("6. Book Ticket\n");
        printf("7. Find My Booking (Hashing)\n");
        printf("8. Recently Viewed Movies\n");
        printf("9. Back\n");
        printf("Enter choice: ");

        if (!getInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: viewMovies(); break;
            case 2: displayMoviesSorted(); break;
            case 3: searchById(); break;
            case 4: searchByName(); break;
            case 5: nearestTheatre(); break;
            case 6: bookTicket(); break;
            case 7: findMyBooking(); break;
            case 8: displayRecentlyViewed(); break;
            case 9: return;
            default: printf("Invalid choice.\n");
        }
    }
}

/* ================================================================
 *                  SAMPLE DATA, MAIN MENU, main()
 * ================================================================ */

/* Sample data so the program can be demonstrated straight away */
void loadSampleData(void) {
    int thane, ghatkopar, chembur, kurla, dadar, vashi;

    catalogAddMovie(3, "Inception", 280);
    catalogAddMovie(1, "Avengers", 250);
    catalogAddMovie(4, "Oppenheimer", 350);
    catalogAddMovie(2, "Interstellar", 300);
    catalogAddMovie(5, "Dune", 320);

    thane     = addTheatre("Galaxy Cinema",   "Thane");
    ghatkopar = addTheatre("Metro Screens",   "Ghatkopar");
    chembur   = addTheatre("Diamond Talkies", "Chembur");
    kurla     = addTheatre("City Plex",       "Kurla");
    dadar     = addTheatre("Sunrise Cinema",  "Dadar");
    vashi     = addTheatre("Harbour Cinemas", "Vashi");

    addRoute(thane,     ghatkopar, 12);
    addRoute(ghatkopar, chembur,    6);
    addRoute(ghatkopar, kurla,      4);
    addRoute(kurla,     chembur,    5);
    addRoute(kurla,     dadar,      7);
    addRoute(dadar,     chembur,    9);
    addRoute(chembur,   vashi,     10);
    addRoute(thane,     vashi,     18);

    assignMovieToTheatre(thane,     1);
    assignMovieToTheatre(thane,     5);
    assignMovieToTheatre(ghatkopar, 2);
    assignMovieToTheatre(ghatkopar, 1);
    assignMovieToTheatre(chembur,   3);
    assignMovieToTheatre(chembur,   4);
    assignMovieToTheatre(kurla,     2);
    assignMovieToTheatre(kurla,     5);
    assignMovieToTheatre(dadar,     3);
    assignMovieToTheatre(dadar,     1);
    assignMovieToTheatre(dadar,     4);
    assignMovieToTheatre(vashi,     4);
}

void freeAll(void) {
    freeHashTable();
    freeGraph();
    freeBST();
    freeRecent();
    freeMovies();
}

void mainMenu(void) {
    int choice;

    while (1) {
        printf("\n===============================\n");
        printf(" SMART MOVIE TICKET BOOKING\n");
        printf("===============================\n");
        printf("1. Admin\n");
        printf("2. User\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        if (!getInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: adminMenu(); break;
            case 2: userMenu(); break;
            case 3: printf("Thank you!\n"); return;
            default: printf("Invalid choice.\n");
        }
    }
}

int main(void) {
    initializeStack();
    initializeQueue();
    initHashTable();
    loadSampleData();

    mainMenu();

    freeAll();
    return 0;
}

/*
 * stage3.h - STAGE 3 : Non-linear data structures, searching, hashing
 *   (A) Binary Search Tree : movie index by name, prefix search, A-Z list
 *   (B) Graph              : theatre network, BFS, DFS, Dijkstra
 *   (C) Hashing            : booking lookup by ID (chaining)
 *   (D) Searching          : linear search vs binary search
 */
#ifndef STAGE3_H
#define STAGE3_H

#include "common.h"
#include "stage2.h"

/* ================================================================
 *        (A) BINARY SEARCH TREE - MOVIE INDEX KEYED BY NAME
 * ----------------------------------------------------------------
 * Every movie in the linked list is also stored here, ordered by
 * name (case-insensitive). Search by name is O(h) and an inorder
 * traversal gives the A-Z listing.
 * ================================================================ */
struct BSTNode {
    int  id;
    char name[NAME_LEN];
    struct BSTNode* left;
    struct BSTNode* right;
};

struct BSTNode* bstRoot = NULL;

static struct BSTNode* bstInsertNode(struct BSTNode* root, int id, const char* name) {
    struct BSTNode* n;
    int c;

    if (root == NULL) {
        n = (struct BSTNode*)malloc(sizeof(struct BSTNode));
        if (!n) {
            printf("Memory allocation failed!\n");
            return NULL;
        }
        n->id = id;
        strncpy(n->name, name, NAME_LEN - 1);
        n->name[NAME_LEN - 1] = '\0';
        n->left = n->right = NULL;
        return n;
    }
    c = cmpNoCase(name, root->name);
    if (c < 0)
        root->left = bstInsertNode(root->left, id, name);
    else if (c > 0)
        root->right = bstInsertNode(root->right, id, name);
    /* c == 0 : duplicate name, ignored (catalogue already blocks this) */
    return root;
}

void bstInsert(int id, const char* name) {
    bstRoot = bstInsertNode(bstRoot, id, name);
}

/* Exact (case-insensitive) search by name. Returns node or NULL. O(h) */
struct BSTNode* bstSearchByName(const char* name) {
    struct BSTNode* cur = bstRoot;
    int c;

    while (cur != NULL) {
        c = cmpNoCase(name, cur->name);
        if (c == 0) return cur;
        cur = (c < 0) ? cur->left : cur->right;
    }
    return NULL;
}

static struct BSTNode* bstMin(struct BSTNode* root) {
    while (root && root->left) root = root->left;
    return root;
}

static struct BSTNode* bstDeleteNode(struct BSTNode* root, const char* name) {
    struct BSTNode* t;
    struct BSTNode* succ;
    int c;

    if (root == NULL) return NULL;

    c = cmpNoCase(name, root->name);
    if (c < 0) {
        root->left = bstDeleteNode(root->left, name);
    } else if (c > 0) {
        root->right = bstDeleteNode(root->right, name);
    } else {
        if (root->left == NULL) {                 /* 0 or 1 child (right) */
            t = root->right;
            free(root);
            return t;
        }
        if (root->right == NULL) {                /* 1 child (left) */
            t = root->left;
            free(root);
            return t;
        }
        /* 2 children: replace with inorder successor */
        succ = bstMin(root->right);
        root->id = succ->id;
        strcpy(root->name, succ->name);
        root->right = bstDeleteNode(root->right, succ->name);
    }
    return root;
}

void bstDelete(const char* name) {
    bstRoot = bstDeleteNode(bstRoot, name);
}

/* Inorder traversal = alphabetical (A-Z) listing */
static void bstInorder(struct BSTNode* root) {
    struct MovieNode* m;

    if (root == NULL) return;
    bstInorder(root->left);
    m = searchMovie(root->id);
    if (m)
        printf("%-5d %-20s Rs.%d\n", m->id, m->name, m->price);
    bstInorder(root->right);
}

void displayMoviesSorted(void) {
    if (bstRoot == NULL) {
        printf("\nNo movies currently available.\n");
        return;
    }
    printf("\n====== MOVIES (A-Z, via BST Inorder) ======\n");
    printf("%-5s %-20s %-10s\n", "ID", "Movie", "Price");
    bstInorder(bstRoot);
}

/* Prefix search (inorder so results come out alphabetically) */
static void prefixSearchNode(struct BSTNode* root, const char* prefix, int* found) {
    struct MovieNode* m;

    if (root == NULL) return;
    prefixSearchNode(root->left, prefix, found);
    if (startsWithNoCase(root->name, prefix)) {
        m = searchMovie(root->id);
        if (m) {
            printf("  ID: %d | Movie: %s | Price: Rs.%d\n", m->id, m->name, m->price);
            (*found)++;
        }
    }
    prefixSearchNode(root->right, prefix, found);
}

/* Prints all movies starting with prefix. Returns how many matched */
int bstPrefixSearch(const char* prefix) {
    int found = 0;
    prefixSearchNode(bstRoot, prefix, &found);
    return found;
}

int bstHeight(struct BSTNode* root) {
    int l, r;
    if (root == NULL) return 0;
    l = bstHeight(root->left);
    r = bstHeight(root->right);
    return 1 + (l > r ? l : r);
}

int bstCount(struct BSTNode* root) {
    if (root == NULL) return 0;
    return 1 + bstCount(root->left) + bstCount(root->right);
}

/* Admin feature: BST info */
void showBSTInfo(void) {
    printf("\n========== MOVIE INDEX (BST) ==========\n");
    printf("Movies indexed : %d\n", bstCount(bstRoot));
    printf("Tree height    : %d\n", bstHeight(bstRoot));
    if (bstRoot)
        printf("Root movie     : %s\n", bstRoot->name);
    displayMoviesSorted();
}

static void bstFreeNode(struct BSTNode* root) {
    if (root == NULL) return;
    bstFreeNode(root->left);
    bstFreeNode(root->right);
    free(root);
}

void freeBST(void) {
    bstFreeNode(bstRoot);
    bstRoot = NULL;
}

/* ================================================================
 *            (B) GRAPH - MULTI-THEATRE NETWORK
 * ----------------------------------------------------------------
 * Vertices = theatres (each in an area / town).
 * Edges    = roads between theatres, weight = distance in km.
 * Stored as an adjacency list (array of theatres, each with a
 * linked list of edges). Undirected and weighted.
 * ================================================================ */
struct Edge {
    int dest;
    int dist;                      /* distance in km */
    struct Edge* next;
};

struct Theatre {
    char name[NAME_LEN];
    char area[NAME_LEN];           /* town / city name */
    struct Edge* adj;
    int  movieIds[MAX_SHOWING];
    int  movieCount;
};

struct Theatre theatres[MAX_THEATRES];
int theatreCount = 0;

int validTheatre(int i) {
    return i >= 0 && i < theatreCount;
}

/* Adds a vertex. Returns its index, or -1 if the graph is full */
int addTheatre(const char* name, const char* area) {
    if (theatreCount >= MAX_THEATRES) {
        printf("Theatre limit reached.\n");
        return -1;
    }
    strncpy(theatres[theatreCount].name, name, NAME_LEN - 1);
    theatres[theatreCount].name[NAME_LEN - 1] = '\0';
    strncpy(theatres[theatreCount].area, area, NAME_LEN - 1);
    theatres[theatreCount].area[NAME_LEN - 1] = '\0';
    theatres[theatreCount].adj = NULL;
    theatres[theatreCount].movieCount = 0;
    return theatreCount++;
}

static int edgeExists(int u, int v) {
    struct Edge* e = theatres[u].adj;
    while (e) {
        if (e->dest == v) return 1;
        e = e->next;
    }
    return 0;
}

static int addEdge(int u, int v, int dist) {
    struct Edge* e = (struct Edge*)malloc(sizeof(struct Edge));
    if (!e) return 0;
    e->dest = v;
    e->dist = dist;
    e->next = theatres[u].adj;
    theatres[u].adj = e;
    return 1;
}

/* Undirected road between two theatres. Returns 1 on success */
int addRoute(int u, int v, int dist) {
    if (!validTheatre(u) || !validTheatre(v)) {
        printf("Invalid theatre number.\n");
        return 0;
    }
    if (u == v) {
        printf("A theatre cannot be connected to itself.\n");
        return 0;
    }
    if (dist <= 0 || dist > MAX_ROUTE_KM) {
        printf("Distance must be between 1 and %d km.\n", MAX_ROUTE_KM);
        return 0;
    }
    if (edgeExists(u, v)) {
        printf("Route already exists.\n");
        return 0;
    }
    if (!addEdge(u, v, dist) || !addEdge(v, u, dist)) {
        printf("Memory allocation failed!\n");
        return 0;
    }
    return 1;
}

int theatreShowsMovie(int t, int movieId) {
    int i;
    for (i = 0; i < theatres[t].movieCount; i++) {
        if (theatres[t].movieIds[i] == movieId) return 1;
    }
    return 0;
}

/* Returns 1 on success */
int assignMovieToTheatre(int t, int movieId) {
    if (!validTheatre(t)) {
        printf("Invalid theatre number.\n");
        return 0;
    }
    if (searchMovie(movieId) == NULL) {
        printf("Movie ID %d not found in catalogue.\n", movieId);
        return 0;
    }
    if (theatreShowsMovie(t, movieId)) {
        printf("Movie is already assigned to this theatre.\n");
        return 0;
    }
    if (theatres[t].movieCount >= MAX_SHOWING) {
        printf("This theatre cannot show more movies.\n");
        return 0;
    }
    theatres[t].movieIds[theatres[t].movieCount++] = movieId;
    return 1;
}

/* Called when a movie is deleted so theatres never reference it */
void removeMovieFromTheatres(int movieId) {
    int t, i;
    for (t = 0; t < theatreCount; t++) {
        for (i = 0; i < theatres[t].movieCount; i++) {
            if (theatres[t].movieIds[i] == movieId) {
                theatres[t].movieIds[i] = theatres[t].movieIds[theatres[t].movieCount - 1];
                theatres[t].movieCount--;
                break;
            }
        }
    }
}

void displayTheatres(void) {
    int t, i;
    struct MovieNode* m;

    if (theatreCount == 0) {
        printf("\nNo theatres available.\n");
        return;
    }
    printf("\n========== THEATRES ==========\n");
    for (t = 0; t < theatreCount; t++) {
        printf("%d. %-12s | %-18s | Showing: ", t, theatres[t].area, theatres[t].name);
        if (theatres[t].movieCount == 0) {
            printf("(none)");
        } else {
            for (i = 0; i < theatres[t].movieCount; i++) {
                m = searchMovie(theatres[t].movieIds[i]);
                if (m) printf("%s%s", m->name, (i < theatres[t].movieCount - 1) ? ", " : "");
            }
        }
        printf("\n");
    }
}

/* Prints the adjacency list */
void displayNetwork(void) {
    int t;
    struct Edge* e;

    if (theatreCount == 0) {
        printf("\nNo theatres available.\n");
        return;
    }
    printf("\n====== THEATRE NETWORK (Adjacency List) ======\n");
    for (t = 0; t < theatreCount; t++) {
        printf("%d. %s -> ", t, theatres[t].area);
        e = theatres[t].adj;
        if (!e) printf("(no routes)");
        while (e) {
            printf("%s(%d km)", theatres[e->dest].area, e->dist);
            if (e->next) printf(", ");
            e = e->next;
        }
        printf("\n");
    }
}

/* Lists the theatres showing a movie. Returns how many */
int listTheatresShowing(int movieId) {
    int t, count = 0;

    for (t = 0; t < theatreCount; t++) {
        if (theatreShowsMovie(t, movieId)) {
            if (count == 0) printf("\nTheatres showing this movie:\n");
            printf("  %d. %-12s | %s\n", t, theatres[t].area, theatres[t].name);
            count++;
        }
    }
    return count;
}

/* ---------------- BFS (uses a simple array queue) ---------------- */
void bfs(int start) {
    int visited[MAX_THEATRES];
    int q[MAX_THEATRES];
    int front = 0, rear = 0;
    int i, u, first = 1;
    struct Edge* e;

    if (!validTheatre(start)) {
        printf("Invalid theatre number.\n");
        return;
    }
    for (i = 0; i < theatreCount; i++) visited[i] = 0;

    printf("\nBFS from %s: ", theatres[start].area);
    visited[start] = 1;
    q[rear++] = start;

    while (front < rear) {
        u = q[front++];
        printf("%s%s", first ? "" : " -> ", theatres[u].area);
        first = 0;
        for (e = theatres[u].adj; e != NULL; e = e->next) {
            if (!visited[e->dest]) {
                visited[e->dest] = 1;
                q[rear++] = e->dest;
            }
        }
    }
    printf("\n");
}

/* ---------------- DFS (recursive) ---------------- */
static void dfsVisit(int u, int visited[], int* first) {
    struct Edge* e;

    visited[u] = 1;
    printf("%s%s", *first ? "" : " -> ", theatres[u].area);
    *first = 0;
    for (e = theatres[u].adj; e != NULL; e = e->next) {
        if (!visited[e->dest])
            dfsVisit(e->dest, visited, first);
    }
}

void dfs(int start) {
    int visited[MAX_THEATRES];
    int i, first = 1;

    if (!validTheatre(start)) {
        printf("Invalid theatre number.\n");
        return;
    }
    for (i = 0; i < theatreCount; i++) visited[i] = 0;

    printf("\nDFS from %s: ", theatres[start].area);
    dfsVisit(start, visited, &first);
    printf("\n");
}

/* ---------------- Dijkstra (array version, O(V^2)) ----------------
 * dist[i] = shortest distance from src to theatre i (INF if unreachable)
 * prev[i] = previous theatre on that shortest path (-1 for none)      */
void dijkstra(int src, int dist[], int prev[]) {
    int visited[MAX_THEATRES];
    int i, count, u, best;
    struct Edge* e;

    for (i = 0; i < theatreCount; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (count = 0; count < theatreCount; count++) {
        /* pick the unvisited theatre with the smallest distance */
        u = -1;
        best = INF;
        for (i = 0; i < theatreCount; i++) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1) break;              /* rest are unreachable */
        visited[u] = 1;

        /* relax all edges going out of u */
        for (e = theatres[u].adj; e != NULL; e = e->next) {
            if (!visited[e->dest] && dist[u] + e->dist < dist[e->dest]) {
                dist[e->dest] = dist[u] + e->dist;
                prev[e->dest] = u;
            }
        }
    }
}

/* Prints the path src -> ... -> dest using area names */
static void printRoute(int prev[], int dest) {
    if (prev[dest] != -1) {
        printRoute(prev, prev[dest]);
        printf(" -> ");
    }
    printf("%s", theatres[dest].area);
}

void shortestRoute(int src, int dest) {
    int dist[MAX_THEATRES], prev[MAX_THEATRES];

    if (!validTheatre(src) || !validTheatre(dest)) {
        printf("Invalid theatre number.\n");
        return;
    }
    dijkstra(src, dist, prev);
    if (dist[dest] == INF) {
        printf("No route from %s to %s.\n", theatres[src].area, theatres[dest].area);
        return;
    }
    printf("\nShortest route: ");
    printRoute(prev, dest);
    printf("\nTotal distance: %d km\n", dist[dest]);
}

/* Finds the nearest theatre (from area src) that shows movieId.
 * Prints the result and returns the theatre index, or -1. */
int findNearestTheatre(int src, int movieId) {
    int dist[MAX_THEATRES], prev[MAX_THEATRES];
    int t, best = -1;

    if (!validTheatre(src)) {
        printf("Invalid area number.\n");
        return -1;
    }
    dijkstra(src, dist, prev);

    for (t = 0; t < theatreCount; t++) {
        if (theatreShowsMovie(t, movieId) && dist[t] != INF) {
            if (best == -1 || dist[t] < dist[best])
                best = t;
        }
    }

    if (best == -1) {
        printf("No reachable theatre is showing this movie.\n");
        return -1;
    }

    printf("\nNearest theatre : %s (%s)\n", theatres[best].name, theatres[best].area);
    if (best == src) {
        printf("It is right in your area (0 km).\n");
    } else {
        printf("Route           : ");
        printRoute(prev, best);
        printf("\nDistance        : %d km\n", dist[best]);
    }
    return best;
}

void freeGraph(void) {
    int t;
    struct Edge* e;
    for (t = 0; t < theatreCount; t++) {
        while (theatres[t].adj != NULL) {
            e = theatres[t].adj;
            theatres[t].adj = e->next;
            free(e);
        }
    }
    theatreCount = 0;
}

/* ================================================================
 *              (C) HASHING - BOOKING LOOKUP BY ID
 * ----------------------------------------------------------------
 * Without hashing, finding a booking by its ID means scanning the
 * whole queue (O(n)). Every confirmed booking is also put in a hash
 * table:   index = bookingId % HASH_SIZE   (division method)
 * Bookings in the same bucket form a linked list (separate
 * chaining). Average lookup time is O(1).
 * ================================================================ */
struct HashNode {
    int key;                       /* booking ID */
    struct Booking* booking;       /* points to the booking in the queue */
    struct HashNode* next;
};

struct HashNode* hashTable[HASH_SIZE];

void initHashTable(void) {
    int i;
    for (i = 0; i < HASH_SIZE; i++)
        hashTable[i] = NULL;
}

/* Division method hash function */
int hashFunction(int key) {
    return key % HASH_SIZE;
}

/* Inserts at the head of the bucket's chain. Returns 1 on success */
int hashInsert(struct Booking* b) {
    int index;
    struct HashNode* node;

    node = (struct HashNode*)malloc(sizeof(struct HashNode));
    if (!node) {
        printf("Memory allocation failed!\n");
        return 0;
    }
    index = hashFunction(b->bookingId);
    node->key = b->bookingId;
    node->booking = b;
    node->next = hashTable[index];
    hashTable[index] = node;
    return 1;
}

/* Looks up a booking by ID. Returns it, or NULL if not found.
 * *comparisons is set to the number of keys checked. */
struct Booking* hashSearch(int bookingId, int* comparisons) {
    struct HashNode* cur;

    *comparisons = 0;
    if (bookingId < 0) return NULL;
    cur = hashTable[hashFunction(bookingId)];
    while (cur != NULL) {
        (*comparisons)++;
        if (cur->key == bookingId) return cur->booking;
        cur = cur->next;
    }
    return NULL;
}

/* Shows every bucket and its chain */
void displayHashTable(void) {
    int i, total = 0, used = 0;
    struct HashNode* cur;

    printf("\n====== BOOKING HASH TABLE (size %d, key %% %d) ======\n",
           HASH_SIZE, HASH_SIZE);
    for (i = 0; i < HASH_SIZE; i++) {
        printf("[%2d] ", i);
        cur = hashTable[i];
        if (cur == NULL) printf("-> NULL");
        else used++;
        while (cur != NULL) {
            printf("-> %d ", cur->key);
            total++;
            cur = cur->next;
        }
        printf("\n");
    }
    printf("Bookings stored: %d | Buckets used: %d of %d\n", total, used, HASH_SIZE);
}

void freeHashTable(void) {
    int i;
    struct HashNode* cur;
    for (i = 0; i < HASH_SIZE; i++) {
        while (hashTable[i] != NULL) {
            cur = hashTable[i];
            hashTable[i] = cur->next;
            free(cur);
        }
    }
}

/* ================================================================
 *          (D) SEARCHING - LINEAR SEARCH vs BINARY SEARCH
 * ----------------------------------------------------------------
 * Linear search : walks the movie linked list node by node, O(n).
 * Binary search : copies the movie IDs into an array, sorts it
 *                 (insertion sort), halves the range each step, O(log n).
 * ================================================================ */

/* Linear search on the movie linked list. Returns 1 if found */
int linearSearchMovie(int id, int* comparisons) {
    struct MovieNode* temp = movieHead;

    *comparisons = 0;
    while (temp != NULL) {
        (*comparisons)++;
        if (temp->id == id) return 1;
        temp = temp->next;
    }
    return 0;
}

/* Sorts arr[0..n-1] in ascending order */
void insertionSort(int arr[], int n) {
    int i, j, key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* Iterative binary search on a sorted array. Returns index or -1 */
int binarySearch(int arr[], int n, int key, int* comparisons) {
    int low = 0, high = n - 1, mid;

    *comparisons = 0;
    while (low <= high) {
        mid = low + (high - low) / 2;
        (*comparisons)++;
        if (arr[mid] == key) return mid;
        if (arr[mid] < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

/* Runs both searches for the same ID and prints the comparisons */
void compareSearches(int id) {
    int n, i, linComp, binComp, linFound, binIndex;
    int* ids;
    struct MovieNode* temp;

    n = countMovies();
    if (n == 0) {
        printf("\nNo movies available.\n");
        return;
    }

    ids = (int*)malloc(n * sizeof(int));
    if (!ids) {
        printf("Memory allocation failed!\n");
        return;
    }
    for (i = 0, temp = movieHead; temp != NULL; temp = temp->next)
        ids[i++] = temp->id;
    insertionSort(ids, n);

    linFound = linearSearchMovie(id, &linComp);
    binIndex = binarySearch(ids, n, id, &binComp);

    printf("\n========== SEARCH COMPARISON ==========\n");
    printf("Sorted movie IDs: ");
    for (i = 0; i < n; i++) printf("%d ", ids[i]);
    printf("\n\n");
    printf("Linear search (linked list): %s, %d comparison(s)\n",
           linFound ? "FOUND" : "NOT FOUND", linComp);
    printf("Binary search (sorted array): %s, %d comparison(s)\n",
           binIndex != -1 ? "FOUND" : "NOT FOUND", binComp);
    if (binIndex != -1)
        printf("Binary search found it at index %d of the sorted array.\n", binIndex);

    free(ids);
}

#endif

/*
 * tree.h - STAGE 3 : Tree, hashing and searching
 *   (A) Binary Search Tree : movie index by name, prefix search, A-Z list
 *   (B) Hashing            : booking lookup by ID (chaining)
 *   (C) Searching          : linear search vs binary search
 */
#ifndef TREE_H
#define TREE_H

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
 *              (B) HASHING - BOOKING LOOKUP BY ID
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
 *          (C) SEARCHING - LINEAR SEARCH vs BINARY SEARCH
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

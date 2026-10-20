/*
 * graph.h - STAGE 3 : Graph - theatre network
 *   Adjacency list, BFS, DFS, Dijkstra shortest route and
 *   nearest theatre showing a movie
 */
#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "stage2.h"

/* ================================================================
 *            GRAPH - MULTI-THEATRE NETWORK
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

#endif

# Smart Movie Ticket Booking System: SLA Stage 3

Group 13: Rishi Ghoricha (13), Aditya Ghosh (14)

There are 4 header files (one per stage) and one `main.c`. You only compile
`main.c`, because it includes the headers. The code is plain **C89**, so it
works in **Turbo C**, gcc, Code::Blocks, Dev-C++ and VS Code.

## Files

| File | Stage | What's inside |
|---|---|---|
| `common.h` | all | Constants, `struct Booking`, safe input helpers |
| `stage1.h` | 1 | **Stack** (seat select / undo), **Queue** (confirmed bookings) |
| `stage2.h` | 2 | **Singly linked lists**: movie catalogue and recently viewed |
| `stage3.h` | 3 | **BST** (movie index), **Graph** (BFS, DFS, Dijkstra), **Hashing** (booking lookup), **Searching** (linear vs binary) |
| `main.c` | - | Booking workflow, admin and user menus, sample data, `main()` |

## How to run

### gcc (Windows / Linux / Mac, also VS Code terminal)

Open a terminal **inside this folder**:

```bash
gcc main.c -o movie_booking
```

Then run `movie_booking.exe` (Windows) or `./movie_booking` (Linux/Mac).

### Turbo C / Turbo C++ (DOSBox)

1. Copy the 5 code files into a folder without spaces, e.g. `C:\TC\BIN\MOVIE`
   (or straight into `C:\TC\BIN`).
2. Open `MAIN.C` and press `Ctrl+F9`. Use `Alt+F5` to see the output.
3. If it says *Unable to open include file*, go to `Options > Directories` and
   add the folder to *Include directories*.

### Code::Blocks / Dev-C++

Open `main.c` and press `F9` (Build and Run). Keep the `.h` files in the same folder.

## Sample data loaded at start

* **Movies:** Inception, Avengers, Oppenheimer, Interstellar, Dune
* **Theatres (graph):** Thane(0), Ghatkopar(1), Chembur(2), Kurla(3), Dadar(4), Vashi(5)
* **Routes (km):** Thane-Ghatkopar 12, Ghatkopar-Chembur 6, Ghatkopar-Kurla 4,
  Kurla-Chembur 5, Kurla-Dadar 7, Dadar-Chembur 9, Chembur-Vashi 10, Thane-Vashi 18

## Quick test checklist (for the tester)

1. **User > 2:** movies come out A-Z (BST inorder).
2. **User > 4**, type `inc`: prefix search finds Inception.
3. **User > 5**, movie `3` (Inception), area `0` (Thane): route
   `Thane -> Ghatkopar -> Chembur`, 18 km (Dijkstra).
4. **User > 6**: book Inception at Chembur, seats 5 6 7, undo once, confirm. You get **Booking ID 1001**.
5. Book Inception at Chembur again and try seat 5: it says it's *already booked*.
6. **User > 7**, ID `1001`: booking found through the hash table.
7. **Admin > 7**: hash table buckets. **Admin > 8**: linear vs binary search comparisons.
8. **Admin > 6 > 6 / 7**: BFS and DFS from Thane. **> 8**: shortest route.
9. **Admin > 2**: adding an existing ID or name (any case) is rejected.
10. **Admin > 3**: delete a movie; it disappears from the BST, theatres and recently viewed.

See `CHANGES.txt` for the list of everything that was changed or added.

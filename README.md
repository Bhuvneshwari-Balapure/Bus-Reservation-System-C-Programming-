# Bus Reservation System (C Programming)

A console-based bus ticket booking system implemented in C.  
It allows user registration, login, booking and cancellation of seats, and maintains persistent storage in text files.

##  Features
- Registers new users and keeps their credentials.
- When a user logs in, a dedicated user file (`<username>.txt`) is created/appended with their activity log.
- Shows a list of available buses (five buses in this version) and the number of available seats.
- For each bus, tracks seat availability and individual bookings in text files (`bus1_seats.txt`, `bus1_status.txt`, etc.).
- Users can book multiple seats in one go, view current seat status, and cancel bookings.
- Each booking / cancellation is recorded in the user’s own file with a timestamp.
- Admin default credentials: `admin` / `adminpass` (configurable).

##  File Structure
- `Bus_Reservation_System.c` — main C source file.
- `users.txt` — stores all registered usernames and passwords (space-separated).
- `busX_seats.txt` — stores remaining number of seats for bus X.
- `busX_status.txt` — lists each seat’s status ("Empty" or passenger name) for bus X.
- `<username>.txt` — for each registered user, records user header and activity log.

##  Technologies Used
- C programming language (console / standard library only).
- File handling (`fopen`, `fprintf`, `fscanf`, `fgets`) to persist data between runs.
- String manipulation and arrays to manage seats and bookings.
- Time and date functions (`time.h`) to apply timestamps for activity logs.

##  How to Build & Run
1. Open your terminal (on Windows: use MSYS2 MinGW64 or similar GCC environment).  
2. Navigate to the project folder:  

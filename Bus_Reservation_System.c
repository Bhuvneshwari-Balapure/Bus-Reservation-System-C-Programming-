/* Bus_Reservation_System.c
   Combined Basic + Advanced features
   Portable C (uses only stdio.h, stdlib.h, string.h, time.h, ctype.h)
   Compile: gcc Bus_Reservation_System.c -o bus.exe
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>


#define MAX_BUSES 5
#define MAX_SEATS 32
#define MAX_NAME_LEN 100
#define USER_FILE "users.txt"
#define CURRENT_USER_LEN 64

/* Bus names (you can change) */
const char *bus_names[MAX_BUSES] = {
    "Manglore Express",
    "Karwar Express",
    "Airavat Express",
    "SeaBird Express",
    "Newport Express"
};

/* Globals */
char current_user[CURRENT_USER_LEN] = {0}; /* holds username after successful login */

/* Function prototypes */
void initialize_files_if_missing();
void main_menu();
void view_bus_list();
void book_tickets();
void cancel_booking();
void show_bus_status();
void show_bus_status_quiet(int bus_no); /* internal use */
int read_available_seats(int bus_no);
void write_available_seats(int bus_no, int seats);
void read_status_into_array(int bus_no, char status[][MAX_NAME_LEN], int *count);
void write_status_from_array(int bus_no, char status[][MAX_NAME_LEN], int count);
void register_user();
int login_user();
void pause_console();
int sanitize_username(const char *in, char *out, size_t outlen);
void append_user_log(const char *fmt, ...);
void get_timestamp(char *buf, size_t len);

int main(void) {
    initialize_files_if_missing();
    main_menu();
    return 0;
}

/* Ensure data files exist for each bus and user file exists */
void initialize_files_if_missing() {
    int i;
    for (i = 1; i <= MAX_BUSES; ++i) {
        char seats_fname[64], status_fname[64];
        snprintf(seats_fname, sizeof(seats_fname), "bus%d_seats.txt", i);
        snprintf(status_fname, sizeof(status_fname), "bus%d_status.txt", i);

        FILE *fs = fopen(seats_fname, "r");
        if (!fs) {
            fs = fopen(seats_fname, "w");
            if (fs) {
                fprintf(fs, "%d\n", MAX_SEATS); /* initially all seats available */
                fclose(fs);
            }
        } else {
            fclose(fs);
        }

        FILE *fst = fopen(status_fname, "r");
        if (!fst) {
            fst = fopen(status_fname, "w");
            if (fst) {
                /* write MAX_SEATS "Empty" entries separated by newlines */
                for (int s = 0; s < MAX_SEATS; ++s) {
                    fprintf(fst, "Empty\n");
                }
                fclose(fst);
            }
        } else {
            fclose(fst);
        }
    }

    /* Ensure user file exists */
    FILE *fu = fopen(USER_FILE, "r");
    if (!fu) {
        fu = fopen(USER_FILE, "w");
        if (fu) {
            /* create a default admin account for convenience */
            fprintf(fu, "admin adminpass\n"); /* username admin, password adminpass */
            fclose(fu);
        }
    } else {
        fclose(fu);
    }
}

/* Pause helper */
void pause_console() {
    printf("\nPress Enter to continue...");
    int c;
    /* consume leftover input until newline */
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/* Main menu with login/registration flow */
void main_menu() {
    int logged_in = 0;
    char choice[8];

    while (1) {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif

        printf("====================================== WELCOME TO BUS RESERVATION SYSTEM ======================================\n\n");
        printf("Please choose an option:\n");
        printf(" 1) Login\n");
        printf(" 2) Register (create new user)\n");
        printf(" 3) View Bus List (no login required)\n");
        printf(" 4) Exit\n");
        printf("\nEnter choice: ");
        if (!fgets(choice, sizeof(choice), stdin)) return;
        int opt = atoi(choice);

        if (opt == 1) {
            logged_in = login_user();
            if (logged_in) {
                /* After login, show user menu */
                int user_opt = 0;
                while (1) {
                    #ifdef _WIN32
                        system("cls");
                    #else
                        system("clear");
                    #endif

                    printf("Logged in as: %s\n", current_user);
                    printf("Choose action:\n");
                    printf(" 1) View Bus List\n");
                    printf(" 2) Book Tickets\n");
                    printf(" 3) Cancel Booking\n");
                    printf(" 4) View Bus Status\n");
                    printf(" 5) Logout\n");
                    printf("Enter choice: ");
                    if (!fgets(choice, sizeof(choice), stdin)) return;
                    user_opt = atoi(choice);
                    if (user_opt == 1) view_bus_list();
                    else if (user_opt == 2) book_tickets();
                    else if (user_opt == 3) cancel_booking();
                    else if (user_opt == 4) show_bus_status();
                    else if (user_opt == 5) {
                        /* append logout entry to user file and clear current_user */
                        if (current_user[0] != '\0') {
                            append_user_log("Logged out");
                            current_user[0] = '\0';
                        }
                        break;
                    }
                    else printf("Invalid choice.\n");
                    pause_console();
                }
            } else {
                printf("Login failed.\n");
                pause_console();
            }
        } else if (opt == 2) {
            register_user();
            pause_console();
        } else if (opt == 3) {
            view_bus_list();
            pause_console();
        } else if (opt == 4) {
            printf("Thank you for using the system!\n");
            break;
        } else {
            printf("Invalid option. Try again.\n");
            pause_console();
        }
    }
}

/* Show the list of buses and available seats */
void view_bus_list() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    printf("=========================================== BUS LIST ============================================\n\n");
    for (int i = 0; i < MAX_BUSES; ++i) {
        int avail = read_available_seats(i+1);
        printf(" [%d] %s   - Available seats: %d\n", i+1, bus_names[i], avail);
    }
}

/* Booking flow */
void book_tickets() {
    char input[16];
    view_bus_list();
    printf("\nEnter bus number to book: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    int bus_no = atoi(input);
    if (bus_no < 1 || bus_no > MAX_BUSES) {
        printf("Invalid bus number.\n");
        return;
    }

    int avail = read_available_seats(bus_no);
    if (avail <= 0) {
        printf("No seats available on this bus.\n");
        return;
    }

    printf("You selected: %s (Available seats: %d)\n", bus_names[bus_no-1], avail);
    printf("How many tickets do you want to book (1-%d)? ", avail);
    if (!fgets(input, sizeof(input), stdin)) return;
    int num = atoi(input);
    if (num < 1 || num > avail) {
        printf("Invalid ticket count.\n");
        return;
    }

    /* load current status into array */
    char status[MAX_SEATS][MAX_NAME_LEN];
    int count = 0;
    read_status_into_array(bus_no, status, &count);

    for (int t = 0; t < num; ++t) {
        int seat_choice = 0;
        printf("\nEnter seat number to book (1-%d): ", MAX_SEATS);
        if (!fgets(input, sizeof(input), stdin)) return;
        seat_choice = atoi(input);
        if (seat_choice < 1 || seat_choice > MAX_SEATS) {
            printf("Invalid seat number. Try again.\n");
            t--; /* retry this ticket */
            continue;
        }
        /* check if seat is empty */
        if (strncmp(status[seat_choice-1], "Empty", 5) != 0) {
            printf("Seat %d is already booked by %s. Choose a different seat.\n", seat_choice, status[seat_choice-1]);
            t--;
            continue;
        }
        printf("Enter passenger name for seat %d: ", seat_choice);
        char name_in[MAX_NAME_LEN];
        if (!fgets(name_in, sizeof(name_in), stdin)) return;
        /* trim newline */
        name_in[strcspn(name_in, "\n")] = '\0';
        if (strlen(name_in) == 0) {
            printf("Name cannot be empty. Try again.\n");
            t--;
            continue;
        }
        strncpy(status[seat_choice-1], name_in, MAX_NAME_LEN-1);
        status[seat_choice-1][MAX_NAME_LEN-1] = '\0';

        printf("Seat %d booked for %s.\n", seat_choice, status[seat_choice-1]);

        /* append booking to current user's file (if logged in) */
        if (current_user[0] != '\0') {
            char msg[256];
            snprintf(msg, sizeof(msg), "Booked: Bus %d Seat %d Name: %s", bus_no, seat_choice, status[seat_choice-1]);
            append_user_log("%s", msg);
        }
    }

    /* update seats count and status file */
    int new_avail = read_available_seats(bus_no) - num;
    write_available_seats(bus_no, new_avail);
    write_status_from_array(bus_no, status, MAX_SEATS);

    printf("\nBooking complete. Total charge: Rs %d\n", 200 * num);
}

/* Cancel booking */
void cancel_booking() {
    char input[16];
    view_bus_list();
    printf("\nEnter bus number for cancellation: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    int bus_no = atoi(input);
    if (bus_no < 1 || bus_no > MAX_BUSES) {
        printf("Invalid bus number.\n");
        return;
    }

    show_bus_status_quiet(bus_no); /* shows seat map */

    printf("\nEnter seat number to cancel: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    int seat_no = atoi(input);
    if (seat_no < 1 || seat_no > MAX_SEATS) {
        printf("Invalid seat number.\n");
        return;
    }

    char status[MAX_SEATS][MAX_NAME_LEN];
    int count = 0;
    read_status_into_array(bus_no, status, &count);

    if (strncmp(status[seat_no-1], "Empty", 5) == 0) {
        printf("Seat %d is already empty.\n", seat_no);
        return;
    }

    printf("Cancelling seat %d booked by %s\n", seat_no, status[seat_no-1]);
    strncpy(status[seat_no-1], "Empty", MAX_NAME_LEN-1);
    status[seat_no-1][MAX_NAME_LEN-1] = '\0';

    write_status_from_array(bus_no, status, MAX_SEATS);
    int new_avail = read_available_seats(bus_no) + 1;
    write_available_seats(bus_no, new_avail);

    /* append cancellation to current user's file (if logged in) */
    if (current_user[0] != '\0') {
        char msg[256];
        snprintf(msg, sizeof(msg), "Cancelled: Bus %d Seat %d", bus_no, seat_no);
        append_user_log("%s", msg);
    }

    printf("Cancellation successful. Rs 200 will be refunded (simulated).\n");
}

/* Show seat status in a nice grid */
void show_bus_status() {
    char input[16];
    view_bus_list();
    printf("\nEnter bus number to view status: ");
    if (!fgets(input, sizeof(input), stdin)) return;
    int bus_no = atoi(input);
    if (bus_no < 1 || bus_no > MAX_BUSES) {
        printf("Invalid bus number.\n");
        return;
    }
    show_bus_status_quiet(bus_no);
}

/* Internal function to print status */
void show_bus_status_quiet(int bus_no) {
    char status[MAX_SEATS][MAX_NAME_LEN];
    int count = 0;
    read_status_into_array(bus_no, status, &count);

    printf("\nBus %d --> %s\n", bus_no, bus_names[bus_no-1]);
    printf("--------------------------------------------------------------------------------\n");
    int index = 0;
    for (int row = 0; row < 8; ++row) {
        printf("\t");
        for (int col = 0; col < 4; ++col) {
            int seat = index + 1;
            printf("%2d.", seat);
            if (strncmp(status[index], "Empty", 5) == 0)
                printf("Empty\t\t");
            else {
                /* print up to 10 chars of name for compact view */
                char shortname[12];
                strncpy(shortname, status[index], 10);
                shortname[10] = '\0';
                printf("%s\t", shortname);
            }
            index++;
        }
        printf("\n");
    }
}

/* Read available seats from file */
int read_available_seats(int bus_no) {
    char fname[64];
    snprintf(fname, sizeof(fname), "bus%d_seats.txt", bus_no);
    FILE *f = fopen(fname, "r");
    if (!f) return 0;
    int x = 0;
    if (fscanf(f, "%d", &x) != 1) x = 0;
    fclose(f);
    return x;
}

/* Write available seats to file */
void write_available_seats(int bus_no, int seats) {
    char fname[64];
    snprintf(fname, sizeof(fname), "bus%d_seats.txt", bus_no);
    FILE *f = fopen(fname, "w");
    if (!f) {
        printf("Error writing seats file for bus %d\n", bus_no);
        return;
    }
    fprintf(f, "%d\n", seats);
    fclose(f);
}

/* Read status file lines into provided array.
   status[i] will contain the string for seat i (0-based).
   count is filled with number of lines read (should be MAX_SEATS). */
void read_status_into_array(int bus_no, char status[][MAX_NAME_LEN], int *count) {
    char fname[64];
    snprintf(fname, sizeof(fname), "bus%d_status.txt", bus_no);
    FILE *f = fopen(fname, "r");
    if (!f) {
        /* create default */
        for (int i = 0; i < MAX_SEATS; ++i) {
            strncpy(status[i], "Empty", MAX_NAME_LEN-1);
            status[i][MAX_NAME_LEN-1] = '\0';
        }
        *count = MAX_SEATS;
        return;
    }
    char line[256];
    int idx = 0;
    while (fgets(line, sizeof(line), f) && idx < MAX_SEATS) {
        /* strip newline */
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) strncpy(status[idx], "Empty", MAX_NAME_LEN-1);
        else strncpy(status[idx], line, MAX_NAME_LEN-1);
        status[idx][MAX_NAME_LEN-1] = '\0';
        idx++;
    }
    /* if file had fewer lines, fill rest with Empty */
    while (idx < MAX_SEATS) {
        strncpy(status[idx], "Empty", MAX_NAME_LEN-1);
        status[idx][MAX_NAME_LEN-1] = '\0';
        idx++;
    }
    fclose(f);
    *count = idx;
}

/* Write status array back to file */
void write_status_from_array(int bus_no, char status[][MAX_NAME_LEN], int count) {
    char fname[64];
    snprintf(fname, sizeof(fname), "bus%d_status.txt", bus_no);
    FILE *f = fopen(fname, "w");
    if (!f) {
        printf("Error writing status file for bus %d\n", bus_no);
        return;
    }
    for (int i = 0; i < count; ++i) {
        fprintf(f, "%s\n", status[i]);
    }
    fclose(f);
}

/* Helper: get timestamp string */
void get_timestamp(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* Append a log message into current user's file with timestamp */
void append_user_log(const char *fmt, ...) {
    if (current_user[0] == '\0') return; /* no logged-in user */
    char user_fname[128];
    snprintf(user_fname, sizeof(user_fname), "%s.txt", current_user);

    FILE *uf = fopen(user_fname, "a");
    if (!uf) return;

    /* timestamp */
    char ts[64];
    get_timestamp(ts, sizeof(ts));
    fprintf(uf, "%s - ", ts);

    /* formatted message */
    va_list args;
    va_start(args, fmt);
    vfprintf(uf, fmt, args);
    va_end(args);

    fprintf(uf, "\n");
    fclose(uf);
}

/* Sanitizes username: keep letters, digits, underscore, hyphen only.
   returns 1 if good, 0 if invalid (empty after sanitize). */
int sanitize_username(const char *in, char *out, size_t outlen) {
    if (!in || !out || outlen == 0) return 0;
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outlen; ++i) {
        char c = in[i];
        if (isalnum((unsigned char)c) || c == '_' || c == '-') {
            out[j++] = c;
        } else {
            /* skip other characters (space, slash, etc.) */
        }
    }
    if (j == 0) {
        out[0] = '\0';
        return 0;
    }
    out[j] = '\0';
    return 1;
}

/* Registration: creates an entry in users.txt and creates username.txt user file */
void register_user() {
    char username_raw[64], password[64], username[64];
    printf("\n===== Register New User =====\n");
    printf("Enter username (letters, digits, _ or - only): ");
    if (!fgets(username_raw, sizeof(username_raw), stdin)) return;
    username_raw[strcspn(username_raw, "\n")] = '\0';

    if (!sanitize_username(username_raw, username, sizeof(username))) {
        printf("Invalid username. Use letters/digits/_/- and avoid spaces.\n");
        return;
    }

    printf("Enter password: ");
    if (!fgets(password, sizeof(password), stdin)) return;
    password[strcspn(password, "\n")] = '\0';

    if (strlen(password) == 0) {
        printf("Password cannot be empty.\n");
        return;
    }

    /* check if user exists in users file */
    FILE *f = fopen(USER_FILE, "r");
    char u[64], p[64];
    while (f && (fscanf(f, "%63s %63s", u, p) == 2)) {
        if (strcmp(u, username) == 0) {
            printf("Username already exists. Choose a different username.\n");
            if (f) fclose(f);
            return;
        }
    }
    if (f) fclose(f);

    /* append to users.txt */
    f = fopen(USER_FILE, "a");
    if (!f) {
        printf("Error opening user database file.\n");
        return;
    }
    fprintf(f, "%s %s\n", username, password);
    fclose(f);

    /* create user's personal file and write header + registration log */
    char user_fname[128];
    snprintf(user_fname, sizeof(user_fname), "%s.txt", username);
    FILE *uf = fopen(user_fname, "a");
    if (!uf) {
        printf("Error creating user file %s\n", user_fname);
        return;
    }
    fprintf(uf, "== User: %s ==\n", username);
    fprintf(uf, "Activity Log:\n");
    fclose(uf);

    /* temporarily set current_user to write registration log */
    strncpy(current_user, username, sizeof(current_user)-1);
    current_user[sizeof(current_user)-1] = '\0';
    append_user_log("Registered");
    /* do not clear current_user here - user still not logged in; keep it blank for safety */
    current_user[0] = '\0';

    printf("Registration successful. You can now login.\n");
}

/* Login: check users.txt for username & password; on success set current_user and log it */
int login_user() {
    char username_raw[64], password[64], username[64];
    printf("\n===== Login =====\n");
    printf("Username: ");
    if (!fgets(username_raw, sizeof(username_raw), stdin)) return 0;
    username_raw[strcspn(username_raw, "\n")] = '\0';

    if (!sanitize_username(username_raw, username, sizeof(username))) {
        printf("Invalid username format.\n");
        return 0;
    }

    printf("Password: ");
    if (!fgets(password, sizeof(password), stdin)) return 0;
    password[strcspn(password, "\n")] = '\0';

    FILE *f = fopen(USER_FILE, "r");
    if (!f) {
        printf("User database missing.\n");
        return 0;
    }
    char u[64], p[64];
    int ok = 0;
    while (fscanf(f, "%63s %63s", u, p) == 2) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            ok = 1;
            break;
        }
    }
    fclose(f);
    if (ok) {
        strncpy(current_user, username, sizeof(current_user)-1);
        current_user[sizeof(current_user)-1] = '\0';
        append_user_log("Logged in");
        printf("Login successful. Welcome, %s!\n", username);
        return 1;
    } else {
        printf("Invalid credentials.\n");
        return 0;
    }
}

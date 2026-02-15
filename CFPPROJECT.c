#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <limits.h>

#include "SQlite\sqlite3.h"
#define max(a, b) ((a) > (b) ? (a) : (b))

sqlite3* db= NULL;

/*===================STRUCTURES===================*/
// Flight structure
typedef struct
{
    int flightId;
    char location[30];
    char destination[30];
    int numberOfPassengers;
    int numberOfSeats;
    char time[30];
} Flight;

// Passenger structure
typedef struct
{
    int ticketID;
    char name[50];
    int age;
    char cnic[15];
    char contactNo[15];
    char email[100];
    int flightID;
    char password[50];
} Passenger;

/*===================HELPER FUNCTION PROTOTYPES===================*/
int input(char *prompt, int mn, int mx);             // HANDLE INT INPUT
void get();                                          // CLEAR SCREEN FUNCTION
void inputString(char *str, int size, char *prompt); // INPUT STRING
void MainMenu();

/*===================DATABASE FUNCTION PROTOTYPES===================*/
int openDatabase(char *filename);
void closeDatabase();
void createTables();
int runQuery(const char *query);
void selectQuery(const char *sql);
int searchRecord(const char *sqlQuery);

/*----------------FLIGHT FUNCTION PROTOTYPES----------------*/
int flightExists(int flightId);
int seatsAvailable(int flightId);
int addFlight(Flight f);
int updatePassengerCount(int flightId, int change);

/*----------------PASSENGER FUNCTION PROTOTYPES----------------*/
int passengerExists(int ticketId);
int addPassenger(Passenger p);
int deletePassenger(int ticketId);

/*----------------Admin Functions Prototypes----------------*/
int passwordCheck();
void AdminMenu();
void FlightsMenu();
void PassengersMenu();
void ManageAccountsMenu();
void addFlightMenu();
void removeFlightMenu();
void updateFlightMenu();
void viewPassengersByFlightMenu();
void viewPassengerDetailsMenu();
void updatePassengerMenu();
void createAdminAccountMenu();
void deleteAdminAccountMenu();
void updateAdminPasswordMenu();

/*----------------Client Functions Prototypes----------------*/
void ClientMenu();
void BookFlightMenu();
void ViewFlightMenu();
void CancelBookingMenu(int ticketId);
int ClientPasswordMenu();

int main()
{
    if (openDatabase("database.db"))
        return -1;
    MainMenu();
    closeDatabase();
}

void MainMenu()
{
    do
    {
        printf("------AIRPORT SYSTEM--------\n");
        printf("---MAIN MENU---\n");
        printf("1. View Flight Menu\n");
        printf("2. Admin\n");
        printf("3. Client\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        char ch = getche();
        printf("\n");

        switch (ch)
        {
        case '1':
            selectQuery("SELECT flightid, location, destination, time FROM flight;");
            break;
        case '2':
            if (passwordCheck())
            {
                printf("WELCOME.....\n");
                get();
                AdminMenu();
            }
            else
            {
                printf("Password Limit Reached\nExiting.....");
                return;
            }
            break;
        case '3':
            ClientMenu();
            break;
        case '4':
            printf("Exiting....\n");
            return;
        default:
            break;
        }

        get();
    } while (1);
}

/*----------------FUNCTION DEFINITIONS----------------------*/
/*===========HELPER FUNCTIONS DEFINATION=============*/
int input(char *prompt, int mn, int mx)
{
    int n = 0;

    do
    {
        printf("%s", prompt);
        if (scanf("%d", &n) == 1)
        {
            while (getchar() != '\n')
                ;
            if (n >= mn && n <= mx)
                return n;
            else if (n < mn)
                printf("Number is Too Small.Enter again....\n");
            else if (n > mx)
                printf("Number is Too Large.Enter again....\n");
            continue;
        }
        printf("Invalid Character\n");
        while (getchar() != '\n')
            ;

    } while (1);
}
void get()
{
    printf("\nEnter any key to continue....");
    getch();
    system("cls");
}
void inputString(char *str, int size, char *prompt)
{
    while (1)
    {
        printf("%s", prompt);

        if (fgets(str, size, stdin) != NULL)
        {
            // Check if the user just pressed Enter (Empty String)
            if (str[0] == '\n')
            {
                printf("Oops! You can't leave this empty. Please try again.\n");
                continue;
            }

            // Find the newline and remove it
            char *ptr = strchr(str, '\n');
            if (ptr)
            {
                *ptr = '\0'; // Replace \n with null terminator
            }
            else
            {
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ;
                printf("Warning: That was a bit long. Enter again...\n");
                continue;
            }

            int hasChar = 0;
            for (int i = 0; str[i] != '\0'; i++)
            {
                if (str[i] != ' ' && str[i] != '\t')
                {
                    hasChar = 1;
                    break;
                }
            }

            if (!hasChar)
            {
                printf("Input can't be just spaces. Type something real!\n");
            }
            else
            {
                return; // Everything looks good!
            }
        }
    }
}
/*----------------DATABASE FUNCTIONS----------------------*/
// open database at the start
int openDatabase(char *filename)
{
    // open database
    int rc = sqlite3_open(filename, &db);

    // check if database opened successfully
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    runQuery("PRAGMA foreign_keys = ON;");
    createTables();
    get();
    return 0;
}
// close database at the end
void closeDatabase()
{
    sqlite3_close(db);
}
// create tables for the first time
void createTables()
{
    int rc = runQuery("CREATE TABLE IF NOT EXISTS flight ("
                      "flightId INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "passengers INT DEFAULT 0,"
                      "seats INTEGER NOT NULL,"
                      "location TEXT NOT NULL,"
                      "destination TEXT NOT NULL,"
                      "time TEXT NOT NULL"
                      ");");
    if (rc == SQLITE_OK)
    {
        printf("Flight table created successfully.\n");
    }
    else
    {
        printf("Failed to create Flight table.\n");
    }

    rc = runQuery("CREATE TABLE IF NOT EXISTS passengers ("
                  "ticketId INTEGER PRIMARY KEY AUTOINCREMENT,"
                  "name TEXT NOT NULL,"
                  "age INTEGER NOT NULL,"
                  "cnic TEXT NOT NULL,"
                  "contact TEXT NOT NULL,"
                  "email TEXT,"
                  "flightId INTEGER NOT NULL,"
                  "password TEXT NOT NULL,"
                  "FOREIGN KEY(flightId) REFERENCES flight(flightId) ON DELETE CASCADE"
                  ");");
    if (rc == SQLITE_OK)
    {
        printf("Passengers table created successfully.\n");
    }
    else
    {
        printf("Failed to create Passengers table.\n");
    }

    rc = runQuery("CREATE TABLE IF NOT EXISTS admin ("
                  "username TEXT PRIMARY KEY,"
                  "password TEXT NOT NULL"
                  ");");

    if (rc == SQLITE_OK)
    {
        printf("Admin table created successfully.\n");
        if (!searchRecord("SELECT * FROM admin;"))
        {

            printf("Default admin account created. Username: admin, Password: admin123\n");
            runQuery("INSERT OR IGNORE INTO admin (username, password) VALUES ('admin', 'admin123');");
        }
    }
    else
    {
        printf("Failed to create Admin table.\n");
    }
}
// we will pass it the query and it will run it
int runQuery(const char *query)
{
    // to store error
    char *err = NULL;

    // execture query
    int rc = sqlite3_exec(db, query, 0, 0, &err);

    // show error
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error running query: %s\n", err);
        sqlite3_free(err);
    }

    return rc;
}
// FOR SELECT QUERY
void selectQuery(const char *sql)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Preparation Error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    int colNum = sqlite3_column_count(stmt);
    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        printf("NO RECORD FOUND.\n");
        sqlite3_finalize(stmt);
        return;
    }
    else
        sqlite3_reset(stmt);
    int colWidth[colNum];
    for (int i = 0; i < colNum; i++)
    {
        colWidth[i] = strlen(sqlite3_column_name(stmt, i));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        for (int i = 0; i < colNum; i++)
        {
            const unsigned char *val = sqlite3_column_text(stmt, i);
            int currSize = strlen(val ? (const char *)val : "NULL");
            colWidth[i] = max(currSize, colWidth[i]);
        }
    }

    sqlite3_reset(stmt);

    for (int i = 0; i < colNum; i++)
    {
        printf("+");
        for (int j = 0; j < colWidth[i] + 2; j++)
        {
            printf("-");
        }
        if (i == colNum - 1)
            printf("+");
    }
    printf("\n| ");
    for (int i = 0; i < colNum; i++)
    {
        printf("%-*s", colWidth[i], sqlite3_column_name(stmt, i));
        printf(" | ");
    }
    printf("\n");

    for (int i = 0; i < colNum; i++)
    {
        printf("+");
        for (int j = 0; j < colWidth[i] + 2; j++)
        {
            printf("-");
        }
        if (i == colNum - 1)
            printf("+");
    }
    printf("\n");

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        printf("| ");
        for (int i = 0; i < colNum; i++)
        {
            const unsigned char *text = sqlite3_column_text(stmt, i);

            switch (sqlite3_column_type(stmt, i))
            {
            case SQLITE_TEXT:
                printf("%-*s", colWidth[i], text ? (char *)text : "NULL");
                break;
            case SQLITE_INTEGER:
                printf("%-*d", colWidth[i], sqlite3_column_int(stmt, i));
                break;
            default:
                printf("%-*s", colWidth[i], text ? (char *)text : "NULL");
                break;
            }

            printf(" | ");
        }
        printf("\n");
    }
    for (int i = 0; i < colNum; i++)
    {
        printf("+");
        for (int j = 0; j < colWidth[i] + 2; j++)
        {
            printf("-");
        }
        if (i == colNum - 1)
            printf("+");
    }
    printf("\n");

    sqlite3_finalize(stmt);

    return;
}
// FOR SEARCHING A RECORD
int searchRecord(const char *sqlQuery)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlQuery, -1, &stmt, NULL);
    // FOUND ANY ERROR
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Preparation Error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    // NO ROW FOUND
    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);

    return 1;
}

/* ================= FLIGHT FUNCTIONS DEFINATION ================= */
int flightExists(int flightId)
{
    char sql[200] = "";
    snprintf(sql, sizeof(sql), "SELECT 1 FROM flight WHERE flightId = %d;", flightId);
    return searchRecord(sql);
}
int seatsAvailable(int flightId)
{
    sqlite3_stmt *stmt;
    const char *sqlQuery = "SELECT seats,passengers FROM flight WHERE flightId = ?;";
    int rc = sqlite3_prepare_v2(db, sqlQuery, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Preparation Error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, flightId);
    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return 0;
    }
    int seats = sqlite3_column_int(stmt, 0);
    int passengers = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    return seats - passengers;
}
int addFlight(Flight f)
{
    char query[500];
    snprintf(query, sizeof(query), "INSERT INTO flight (seats, passengers, location, destination, time) VALUES (%d, %d, '%s', '%s', '%s');", f.numberOfSeats, f.numberOfPassengers, f.location, f.destination, f.time);
    return runQuery(query);
}
int updatePassengerCount(int flightId, int change)
{
    char query[200];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT passengers,seats FROM flight WHERE flightId = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, flightId);
    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return -1;
    }
    int currentPassengers = sqlite3_column_int(stmt, 0);
    int totalSeats = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
    if (currentPassengers + change < 0 || currentPassengers + change > totalSeats)
    {
        return -1;
    }
    snprintf(query, sizeof(query), "UPDATE flight SET passengers = passengers + (%d) WHERE flightId = %d;", change, flightId);
    return runQuery(query);
}
int deleteFlight(int flightId)
{
    char query[200];
    snprintf(query, sizeof(query), "DELETE FROM flight WHERE flightId = %d;", flightId);
    int rc = runQuery(query);
    if (rc != SQLITE_OK)
    {
        printf("Error deleting flight\n");
        return rc;
    }

    printf("Flight Deleted Successfuly\n");
    return rc;
}
/* ================= PASSENGER FUNCTIONS DEFINATION ================= */
int passengerExists(int ticketId)
{
    char sql[200] = "";
    snprintf(sql, sizeof(sql), "SELECT 1 FROM passengers WHERE ticketId = %d;", ticketId);
    return searchRecord(sql);
}
int addPassenger(Passenger p)
{
    char query[600];
    snprintf(query, sizeof(query), "INSERT INTO passengers (name, age, cnic, contact, email, flightId, password) VALUES ('%s', %d, '%s', '%s', '%s', %d, '%s');", p.name, p.age, p.cnic, p.contactNo, p.email, p.flightID, p.password);
    int rc = runQuery(query);
    if (rc != SQLITE_OK)
    {
        return rc;
    }
    return updatePassengerCount(p.flightID, 1) != SQLITE_OK;
}
int deletePassenger(int ticketId)
{
    char query[200];
    snprintf(query, sizeof(query), "DELETE FROM passengers WHERE ticketId = %d;", ticketId);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT flightId FROM passengers WHERE ticketId = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, ticketId);
    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return -1;
    }
    int flightId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    int rc = runQuery(query);
    if (rc != SQLITE_OK)
    {
        return rc;
    }
    return updatePassengerCount(flightId, -1) != SQLITE_OK;
}

/* ================= ADMIN MENU FUNCTIONS DEFINATION ================= */
// TAKE INPUT USERNAME AND PASSWORD
int passwordCheck()
{
    for (int i = 0; i < 3; i++)
    {
        char username[50];
        char inputPassword[50];

        inputString(username, 50, "Enter username: ");
        inputString(inputPassword, 50, "Enter password: ");

        char sqlQuery[200];
        snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM admin WHERE username = '%s' and password = '%s';", username, inputPassword);
        if (searchRecord(sqlQuery))
            return 1;
        else
            printf("Wrong username or Password\n");
    }
    return 0;
}
void AdminMenu()
{
    char ch;
    do
    {
        printf("\n--- Admin Menu ---\n");
        printf("1. Manage Flights\n");
        printf("2. Manage Passengers\n");
        printf("3. Manage Accounts\n");
        printf("4. Logout\n");
        printf("Enter your choice: ");
        ch = getche();
        printf("\n");

        switch (ch)
        {
        case '1':
            FlightsMenu();
            break;
        case '2':
            PassengersMenu();
            break;
        case '3':
            ManageAccountsMenu();
            break;
        case '4':
            printf("Logging out...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
        }

        get();
    } while (ch != '4');
}
void FlightsMenu()
{
    system("cls");
    char choice;
    do
    {
        printf("\n--- Flights Management Menu ---\n");
        printf("1. View All Flights\n");
        printf("2. Add New Flight\n");
        printf("3. Remove Flight\n");
        printf("4. Update Flight\n");
        printf("5. Back to Admin Menu\n");
        printf("Enter your choice: ");
        choice = getche();
        printf("\n");

        switch (choice)
        {
        case '1':
            selectQuery("SELECT * FROM flight;");
            break;
        case '2':
            addFlightMenu();
            break;
        case '3':
            if (searchRecord("SELECT flightid FROM flight;"))
                removeFlightMenu();
            else
                printf("No Flights to REMOVE\n");
            break;
        case '4':
            if (searchRecord("SELECT flightid FROM flight;"))
                updateFlightMenu();
            else
                printf("No Flights to UPDATE\n");
            break;
        case '5':
            printf("Returning to Admin Menu...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
        }
        get();
    } while (choice != '5');
}
void addFlightMenu()
{
    Flight f;

    inputString(f.location, sizeof(f.location), "Enter location: ");
    inputString(f.destination, sizeof(f.destination), "Enter destination: ");
    f.numberOfSeats = input("Enter Number of Seats: ", 0, 300);
    inputString(f.time, sizeof(f.time), "Enter Time (e.g., 2023-10-15 14:30): ");
    f.numberOfPassengers = 0;

    if (addFlight(f) == SQLITE_OK)
    {
        printf("Flight added successfully.\n");
    }
    else
    {
        printf("Failed to add flight.\n");
    }
}
void removeFlightMenu()
{
    int flightId = input("Enter Flight ID to remove: ", 1, INT_MAX);

    if (!flightExists(flightId))
    {
        printf("Flight with ID %d does not exist.\n", flightId);
        return;
    }

    char query[200];
    snprintf(query, sizeof(query), "DELETE FROM flight WHERE flightId = %d;", flightId);
    if (runQuery(query) == SQLITE_OK)
    {
        printf("Flight removed successfully.\n");
    }
    else
    {
        printf("Failed to remove flight.\n");
    }
}
void updateFlightMenu()
{
    int flightId = input("Enter Flight ID to update: ", 1, INT_MAX);

    if (!flightExists(flightId))
    {
        printf("Flight with ID %d does not exist.\n", flightId);
        return;
    }

    char ch;
    do
    {
        system("cls");
        printf("---UPDATE FLIGHT MENU id=%d\n", flightId);
        printf("OLDER DATA\n");
        char query[200];
        snprintf(query, sizeof(query), "SELECT * FROM flight WHERE flightid = %d", flightId);
        selectQuery(query);

        printf("1. Update Location\n");
        printf("2. Update Destination\n");
        printf("3. Update Number of Seats\n");
        printf("4. Update Time\n");
        printf("5. Back to Flights Menu\n");
        printf("Enter your choice: ");
        ch = getche();
        char location[30];
        char destination[30];
        char time[30];
        int numberOfSeats;
        printf("\n");
        switch (ch)
        {
        case '1':
            inputString(location, 30, "Enter new location: ");

            snprintf(query, sizeof(query), "UPDATE flight SET location = '%s' WHERE flightId = %d;", location, flightId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Location updated successfully.\n");
            }
            else
            {
                printf("Failed to update location.\n");
            }
            break;
        case '2':
            inputString(destination, 30, "Enter new Destination: ");
            snprintf(query, sizeof(query), "UPDATE flight SET destination = '%s' WHERE flightId = %d;", destination, flightId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Destination updated successfully.\n");
            }
            else
            {
                printf("Failed to update destination.\n");
            }
            break;
        case '3':
            numberOfSeats = input("Enter new Number of Seats: ", 1, INT_MAX);

            snprintf(query, sizeof(query), "UPDATE flight SET seats = %d WHERE flightId = %d;", numberOfSeats, flightId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Number of Seats updated successfully.\n");
            }
            else
            {
                printf("Failed to update number of seats.\n");
            }
            break;
        case '4':
            inputString(time, 30, "Enter new Time (e.g., 2023-10-15 14:30): ");

            snprintf(query, sizeof(query), "UPDATE flight SET time = '%s' WHERE flightId = %d;", time, flightId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Time updated successfully.\n");
            }
            else
            {
                printf("Failed to update time.\n");
            }
            break;
        case '5':
            printf("Returning to Flights Menu...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
        get();
    } while (1);
}
/* ================= ADMIN PASSENGERS FUNCTIONS DEFINATION ================= */
void PassengersMenu()
{
    system("cls");
    char ch;
    do
    {
        printf("\n--- Passengers Management Menu ---\n");
        printf("1. View All Passengers\n");
        printf("2. View Passengers by Flight\n");
        printf("3. View Passenger Details\n");
        printf("4. Update Passenger\n");
        printf("5. Back to Admin Menu\n");
        printf("Enter your choice: ");
        ch = getche();
        printf("\n");

        switch (ch)
        {
        case '1':
            selectQuery("SELECT ticketId,name,contact,flightid FROM passengers;");
            break;
        case '2':
            viewPassengersByFlightMenu();
            break;
        case '3':
            viewPassengerDetailsMenu();
            break;
        case '4':
            updatePassengerMenu();
            break;
        case '5':
            printf("Returning to Admin Menu...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }

        get();
    } while (1);
}
void viewPassengersByFlightMenu()
{
    int flightId = input("Enter Flight ID: ", 1, INT_MAX);

    if (!flightExists(flightId))
    {
        printf("Flight with ID %d does not exist.\n", flightId);
        return;
    }

    char query[200];
    snprintf(query, sizeof(query), "SELECT ticketId,name,contact,flightId FROM passengers WHERE flightId = %d;", flightId);
    selectQuery(query);
}
void viewPassengerDetailsMenu()
{
    int ticketId = input("Enter Ticket ID: ", 1, INT_MAX);

    if (!passengerExists(ticketId))
    {
        printf("Passenger with Ticket ID %d does not exist.\n", ticketId);
        return;
    }

    char query[200];
    snprintf(query, sizeof(query), "SELECT ticketId,name,age,cnic,contact,email,flightId FROM passengers WHERE ticketId = %d;", ticketId);
    selectQuery(query);
}
void updatePassengerMenu()
{
    system("cls");
    int ticketId = input("Enter Ticket ID to update: ", 1, INT_MAX);

    if (!passengerExists(ticketId))
    {
        printf("Passenger with Ticket ID %d does not exist.\n", ticketId);
        return;
    }

    char ch;
    do
    {
        printf("OLD DATA\n");
        char query[200];
        snprintf(query, sizeof(query), "SELECT * FROM passengers WHERE ticketId = %d", ticketId);
        selectQuery(query);

        printf("1. Update Name\n");
        printf("2. Update Age\n");
        printf("3. Update CNIC\n");
        printf("4. Update Contact Number\n");
        printf("5. Update Email\n");
        printf("6. Back to Passengers Menu\n");
        printf("Enter your choice: ");
        ch = getche();
        printf("\n");
        char email[100];
        char name[50];
        int age ;
        char contactNo[12];
        char cnic[14];

        switch (ch)
        {
        case '1':
            inputString(name, 50, "Enter new name: ");

            snprintf(query, sizeof(query), "UPDATE passengers SET name = '%s' WHERE ticketId = %d;", name, ticketId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Name updated successfully.\n");
            }
            else
            {
                printf("Failed to update name.\n");
            }
            break;
        case '2':
            age = input("Enter new Age: ", 18, 150);

            snprintf(query, sizeof(query), "UPDATE passengers SET age = %d WHERE ticketId = %d;", age, ticketId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Age updated successfully.\n");
            }
            else
            {
                printf("Failed to update age.\n");
            }
            break;
        case '3':
            inputString(cnic, sizeof(cnic), "Enter cnic (without dashes): ");

            snprintf(query, sizeof(query), "UPDATE passengers SET cnic = '%s' WHERE ticketId = %d;", cnic, ticketId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("CNIC updated successfully.\n");
            }
            else
            {
                printf("Failed to update CNIC.\n");
            }
            break;
        case '4':
            inputString(contactNo, sizeof(contactNo), "Enter number (03---------): ");

            snprintf(query, sizeof(query), "UPDATE passengers SET contact = '%s' WHERE ticketId = %d;", contactNo, ticketId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Contact Number updated successfully.\n");
            }
            else
            {
                printf("Failed to update Contact Number.\n");
            }
            break;
        case '5':
            inputString(email, 100, "Enter email: ");

            snprintf(query, sizeof(query), "UPDATE passengers SET email = '%s' WHERE ticketId = %d;", email, ticketId);
            if (runQuery(query) == SQLITE_OK)
            {
                printf("Email updated successfully.\n");
            }
            else
            {
                printf("Failed to update Email.\n");
            }
            break;
        case '6':
            printf("Returning to Passengers Menu...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }

        get();
    } while (ch != '6');
}

/* ================= ADMIN ACCOUNT FUNCTIONS DEFINATION ================= */
void ManageAccountsMenu()
{
    system("cls");
    char ch;
    do
    {
        printf("\n--- Manage Accounts Menu ---\n");
        printf("1. View all admin\n");
        printf("2. Add Admin Account\n");
        printf("3. Delete Admin Account\n");
        printf("4. Update Admin Password\n");
        printf("5. Back to Admin Menu\n");
        printf("Enter your choice: ");
        ch = getche();
        printf("\n");

        switch (ch)
        {
        case '1':
            selectQuery("SELECT * FROM admin;");
            break;
        case '2':
            createAdminAccountMenu();
            break;
        case '3':
            deleteAdminAccountMenu();
            break;
        case '4':
            updateAdminPasswordMenu();
            break;
        case '5':
            printf("Returning to Admin Menu...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }

        get();
    } while (1);
}
void createAdminAccountMenu()
{
    char username[50];
    char password[50];

    inputString(username, 50, "Enter username: ");
    inputString(password, 50, "Enter new password: ");

    char sql[200];
    snprintf(sql, 200, "SELECT * FROM admin WHERE username = '%s'", username);
    if (searchRecord(sql))
    {
        printf("Account already exist.Change username...\n");
        return;
    }

    snprintf(sql, sizeof(sql), "INSERT INTO admin (username, password) VALUES ('%s', '%s');", username, password);
    if (runQuery(sql) == SQLITE_OK)
    {
        printf("Admin account created successfully.\n");
    }
    else
    {
        printf("Failed to create admin account.\n");
    }
}
void deleteAdminAccountMenu()
{
    char username[50];
    inputString(username, sizeof(username), "Enter username to DELETE: ");

    // CHECK IF USERNAME EXISTS
    char sql[200];
    snprintf(sql, 200, "SELECT * FROM admin WHERE username = '%s'", username);
    if (!searchRecord(sql))
    {
        printf("No Account found with this username.\n");
        return;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM admin WHERE username = '%s';", username);
    if (runQuery(sql) == SQLITE_OK)
    {
        printf("Admin account deleted successfully.\n");
    }
    else
    {
        printf("Failed to delete admin account.\n");
    }
}
void updateAdminPasswordMenu()
{
    char username[50];
    char newPassword[50];

    inputString(username, sizeof(username), "Enter username to UPDATE: ");

    // CHECK IF USERNAME EXISTS
    char sql[200];
    snprintf(sql, 200, "SELECT * FROM admin WHERE username = '%s'", username);
    if (!searchRecord(sql))
    {
        printf("No Account found with this username.\n");
        return;
    }

    inputString(newPassword, sizeof(newPassword), "Enter new password: ");

    snprintf(sql, sizeof(sql), "UPDATE admin SET password = '%s' WHERE username = '%s';", newPassword, username);
    if (runQuery(sql) == SQLITE_OK)
    {
        printf("Admin password updated successfully.\n");
    }
    else
    {
        printf("Failed to update admin password.\n");
    }
}

/* ================= CLIENT PANEL FUNCTIONS DEFINATION ================= */
void ClientMenu()
{
    system("cls");
    char ch;
    do
    {
        printf("\n--- Client Menu ---\n");
        printf("1. Book a Flight\n");
        printf("2. View Flight Details\n");
        printf("3. Cancel Booking\n");
        printf("4. View All Flights\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        ch = getche();
        printf("\n");
        int ticketId;
        switch (ch)
        {
        case '1':
            BookFlightMenu();
            break;
        case '2':
            ViewFlightMenu();
            break;
        case '3':
            ticketId = ClientPasswordMenu();
            CancelBookingMenu(ticketId);
            break;
        case '4':
            selectQuery("SELECT * FROM flight;");
            break;
        case '5':
            printf("Exiting...\n");
            return;
        default:
            printf("Invalid choice. Please try again.\n");
        }

        get();
    } while (1);
}
void ViewFlightMenu()
{
    int flightId = input("Enter Flight ID to view details: ", 1, INT_MAX);

    if (!flightExists(flightId))
    {
        printf("Flight with ID %d does not exist.\n", flightId);
        return;
    }

    char query[200];
    snprintf(query, sizeof(query), "SELECT * FROM flight WHERE flightId = %d;", flightId);
    selectQuery(query);
}
int ClientPasswordMenu()
{
    for (int i = 0; i < 3; i++)
    {
        // INPUT ID
        int id = input("Enter your Ticket ID: ", 1, INT_MAX);
        if (!passengerExists(id))
        {
            printf("Passenger with Ticket ID %d does not exist.\n", id);
        }

        // INPUT PASSWORD
        char inputPassword[50];
        inputString(inputPassword, sizeof(inputPassword), "Enter password: ");

        // CHECK ANY ACCOUNT WITH ID AND PASSWORD
        char sql[200];
        snprintf(sql, sizeof(sql), "SELECT * FROM passengers WHERE ticketId = %d and password = '%s';", id, inputPassword);

        // RETURN ID FOR PROCESSING
        if (searchRecord(sql))
        {
            return id;
        }
    }

    return 0;
}
void BookFlightMenu()
{
    char location[30];
    char destination[30];

    inputString(location, sizeof(location), "Enter location: ");
    inputString(destination, sizeof(destination), "Enter destination: ");

    char query[300];
    snprintf(query, sizeof(query), "SELECT * FROM flight WHERE location LIKE '%s' AND destination LIKE '%s';", location, destination);
    if (searchRecord(query))
        selectQuery(query);
    else
    {
        printf("SORRY No Planes from %s to %s", location, destination);
        return;
    }

    int flightId = input("Enter Flight ID to book: ", 1, INT_MAX);

    if (!flightExists(flightId))
    {
        printf("Flight with ID %d does not exist.\n", flightId);
        return;
    }
    if (seatsAvailable(flightId) <= 0)
    {
        printf("No seats available on Flight ID %d.\n", flightId);
        return;
    }

    Passenger p;
    p.flightID = flightId;
    inputString(p.name, sizeof(p.name), "Enter name: ");
    p.age = input("Enter Age: ", 18, 120);
    inputString(p.cnic, sizeof(p.cnic), "Enter cnic (without dashes): ");
    // SEARCH IF THE PASSENGER ALREADY IN THAT PLANE
    snprintf(query, sizeof(query), "SELECT * FROM passengers WHERE cnic = '%s'", p.cnic);
    if (searchRecord(query))
    {
        printf("Entity Already Present With That CNIC\n");
        return;
    }

    inputString(p.contactNo, sizeof(p.contactNo), "Enter Contact (without dashes): ");
    inputString(p.email, sizeof(p.email), "Enter email: ");
    inputString(p.password, sizeof(p.password), "Enter password: ");

    if (addPassenger(p) == SQLITE_OK)
    {
        printf("Booking successful.\n");
        snprintf(query, sizeof(query), "SELECT * from passengers WHERE cnic = '%s';", p.cnic);
        selectQuery(query);
    }
    else
    {
        printf("Failed to book flight.\n");
    }

}
void CancelBookingMenu(int ticketId)
{
    if (deletePassenger(ticketId) == SQLITE_OK)
    {
        printf("Booking cancelled successfully.\n");
    }
    else
    {
        printf("Failed to cancel booking.\n");
    }
}

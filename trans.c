// Bank-account program: Advanced Architectural Deep Update
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define FILE_NAME "credit_deep.dat"

// --- ADVANCED DATA STRUCTURES ---

// Enum for readable menu options
typedef enum {
    MENU_TEXT_FILE = 1,
    MENU_UPDATE,
    MENU_ADD,
    MENU_DELETE,
    MENU_LIST,
    MENU_SEARCH,
    MENU_SORT,
    MENU_TRANSFER,
    MENU_EXIT
} MenuOption;

// Typedef struct for cleaner references
// VARIABLES EXPLANATION: A struct is a custom composite variable type. It groups 
// multiple primitive variables (like unsigned int, double, char array) together 
// so they can be handled as a single logical unit representing an account.
typedef struct {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
    char mobileNumber[15];
    char accountType[10];
} ClientData;

// --- UTILITY & VALIDATION FUNCTIONS ---

void clearInputBuffer(void) {
    int c;
    // LOOPS EXPLANATION: This 'while' loop executes repeatedly until it consumes 
    // a newline character or EOF. It is essential for clearing standard input.
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void clearScreen(void) {
    // Attempt to clear screen for cleaner UI
    system("cls || clear");
}

void logTransaction(const char* action, unsigned int acctNum, double amount) {
    FILE *logPtr = fopen("transactions.log", "a");
    if (logPtr != NULL) {
        time_t now = time(NULL);
        char *date = ctime(&now);
        date[strlen(date)-1] = '\0'; // remove newline
        fprintf(logPtr, "[%s] Action: %-15s | Acct: %-6u | Amount: %10.2f\n", date, action, acctNum, amount);
        fclose(logPtr);
    }
}

int isValidName(const char* name) {
    if (strlen(name) == 0) return 0;
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha(name[i]) && name[i] != '-') return 0;
    }
    return 1;
}

int isValidMobile(const char* mobile) {
    if (strlen(mobile) < 10) return 0;
    for (int i = 0; mobile[i] != '\0'; i++) {
        if (!isdigit(mobile[i])) return 0;
    }
    return 1;
}

void printTableHeader(void) {
    printf("\n======================================================================================\n");
    printf("| %-6s | %-15s | %-15s | %-13s | %-9s | %-10s |\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
    printf("======================================================================================\n");
}

void printTableFooter(void) {
    printf("======================================================================================\n\n");
}

// POINTER EXPLANATION: We pass 'const ClientData *client' as a pointer instead of by value.
// This means we only pass an 8-byte memory address rather than cloning all 60+ bytes of the struct. 
// This vastly improves processing speed and memory usage.
void displayRecord(const ClientData *client) {
    // Structure access via pointers uses the -> operator instead of the . operator
    printf("| %-6u | %-15s | %-15s | %-13s | %-9s | %10.2f |\n", 
           client->acctNum, client->lastName, client->firstName, 
           client->mobileNumber, client->accountType, client->balance);
}

// Comparator for qsort
int compareByBalance(const void *a, const void *b) {
    ClientData *clientA = (ClientData *)a;
    ClientData *clientB = (ClientData *)b;
    // Descending order
    if (clientB->balance > clientA->balance) return 1;
    if (clientB->balance < clientA->balance) return -1;
    return 0;
}

// --- CORE PROTOTYPES ---
MenuOption enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAllRecords(FILE *fPtr);
void searchAccount(FILE *fPtr);
void sortAndDisplay(FILE *fPtr);
void transferBalance(FILE *fPtr);

// --- MAIN ---
int main(int argc, char *argv[])
{
    FILE *cfPtr;
    MenuOption choice;

    if ((cfPtr = fopen(FILE_NAME, "rb+")) == NULL) {
        if ((cfPtr = fopen(FILE_NAME, "wb+")) != NULL) {
            ClientData blankClient = {0, "", "", 0.0, "", ""};
            for (int i = 0; i < MAX_ACCOUNTS; i++) {
                fwrite(&blankClient, sizeof(ClientData), 1, cfPtr);
            }
            rewind(cfPtr);
        } else {
            fprintf(stderr, "%s: Critical Error. File could not be opened.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    clearScreen();
    printf("--- Welcome to the Advanced Banking System ---\n");

    // LOOPS EXPLANATION: This while loop acts as the primary program loop. 
    // It keeps displaying the menu until the user specifically chooses MENU_EXIT.
    while ((choice = enterChoice()) != MENU_EXIT) {
        clearScreen();
        // SWITCH-CASE EXPLANATION: A switch statement checks the integer 'choice' 
        // against multiple cases. It provides much cleaner syntax and better 
        // performance than a long chain of if-else statements for menus.
        switch (choice) {
            case MENU_TEXT_FILE: textFile(cfPtr); break;
            case MENU_UPDATE: updateRecord(cfPtr); break;
            case MENU_ADD: newRecord(cfPtr); break;
            case MENU_DELETE: deleteRecord(cfPtr); break;
            case MENU_LIST: listAllRecords(cfPtr); break;
            case MENU_SEARCH: searchAccount(cfPtr); break;
            case MENU_SORT: sortAndDisplay(cfPtr); break;
            case MENU_TRANSFER: transferBalance(cfPtr); break;
            default: fprintf(stderr, "Incorrect choice. Please try again.\n"); break;
        }
    }

    printf("Exiting system. Have a great day!\n");
    fclose(cfPtr);
    return EXIT_SUCCESS;
}

// --- IMPLEMENTATIONS ---

MenuOption enterChoice(void) {
    unsigned int menuChoice;
    printf("\n==================================\n");
    printf("             MAIN MENU            \n");
    printf("==================================\n");
    printf("1 - Export Accounts to Text File\n");
    printf("2 - Update an Account Balance\n");
    printf("3 - Add a New Account\n");
    printf("4 - Delete an Account\n");
    printf("5 - List All Active Accounts\n");
    printf("6 - Search Account by Last Name\n");
    printf("7 - Sort and Display by Balance\n");
    printf("8 - Transfer Balance\n");
    printf("9 - End Program\n");
    printf("==================================\n");
    printf("Enter choice: ");

    if (scanf("%u", &menuChoice) != 1) {
        clearInputBuffer();
        return 0;
    }
    return (MenuOption)menuChoice;
}

void textFile(FILE *readPtr) {
    FILE *writePtr;
    ClientData client = {0, "", "", 0.0, "", ""};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) {
        fprintf(stderr, "Error: Could not open accounts.txt for writing.\n");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s %-15s %-15s %-13s %-9s %10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
    fprintf(writePtr, "----------------------------------------------------------------------------------\n");

    while (fread(&client, sizeof(ClientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6u %-15s %-15s %-13s %-9s %10.2f\n", 
                    client.acctNum, client.lastName, client.firstName, 
                    client.mobileNumber, client.accountType, client.balance);
        }
    }
    fclose(writePtr);
    printf("Success: Exported active records to 'accounts.txt'.\n");
}

void updateRecord(FILE *fPtr) {
    unsigned int account;
    double transaction;
    ClientData client = {0, "", "", 0.0, "", ""};

    printf("Enter account to update (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1) {
        fprintf(stderr, "Invalid input. Returning to menu.\n");
        clearInputBuffer();
        return;
    }
    if (account < 1 || account > MAX_ACCOUNTS) {
        fprintf(stderr, "Invalid account number. Range is 1 - %d.\n", MAX_ACCOUNTS);
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        fprintf(stderr, "Account #%u has no information.\n", account);
        return;
    }

    printTableHeader();
    displayRecord(&client);
    printTableFooter();

    printf("Enter charge (-) or payment (+): ");
    if (scanf("%lf", &transaction) != 1) {
        fprintf(stderr, "Invalid transaction amount.\n");
        clearInputBuffer();
        return;
    }
    
    // CONDITIONAL EXPLANATION: The if-else block directs program flow based on boolean logic. 
    // Here it serves as the Negative Balance Restriction safeguard.
    if (client.balance + transaction < 0) {
        fprintf(stderr, "Transaction declined! Insufficient balance.\n");
    } else {
        client.balance += transaction;
        printf("Transaction successful. New balance: %.2f\n", client.balance);
        
        fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
        fwrite(&client, sizeof(ClientData), 1, fPtr);
        
        logTransaction(transaction >= 0 ? "Deposit" : "Withdrawal", client.acctNum, transaction);
    }
}

void deleteRecord(FILE *fPtr) {
    ClientData client;
    ClientData blankClient = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter account number to delete (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1) {
        fprintf(stderr, "Invalid input.\n");
        clearInputBuffer();
        return;
    }
    if (accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        fprintf(stderr, "Invalid account number.\n");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        fprintf(stderr, "Account %u does not exist.\n", accountNum);
    } else {
        fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
        fwrite(&blankClient, sizeof(ClientData), 1, fPtr);
        printf("Account %u successfully deleted.\n", accountNum);
        
        logTransaction("Account Deleted", accountNum, 0.0);
    }
}

void newRecord(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter new account number (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1) {
        fprintf(stderr, "Invalid input.\n");
        clearInputBuffer();
        return;
    }
    if (accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        fprintf(stderr, "Invalid account number.\n");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum != 0) {
        fprintf(stderr, "Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter First Name: ");
    scanf("%9s", client.firstName);
    if (!isValidName(client.firstName)) {
        fprintf(stderr, "Invalid Name. Must be alphabetic.\n");
        return;
    }

    printf("Enter Last Name: ");
    scanf("%14s", client.lastName);
    if (!isValidName(client.lastName)) {
        fprintf(stderr, "Invalid Name. Must be alphabetic.\n");
        return;
    }

    printf("Enter Mobile Number (10 digits): ");
    scanf("%14s", client.mobileNumber);
    if (!isValidMobile(client.mobileNumber)) {
        fprintf(stderr, "Invalid Mobile. Must be numeric and at least 10 digits.\n");
        return;
    }

    printf("Enter Account Type (Savings/Current): ");
    scanf("%9s", client.accountType);

    printf("Enter Initial Balance: ");
    if (scanf("%lf", &client.balance) != 1 || client.balance < 0) {
        fprintf(stderr, "Invalid Balance.\n");
        clearInputBuffer();
        return;
    }

    client.acctNum = accountNum;
    fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
    fwrite(&client, sizeof(ClientData), 1, fPtr);
    printf("Success: Account %u created.\n", accountNum);
    
    logTransaction("Account Created", accountNum, client.balance);
}

void listAllRecords(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", ""};
    int found = 0;

    printTableHeader();
    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            displayRecord(&client);
            found = 1;
        }
    }
    printTableFooter();

    if (!found) {
        printf("No active accounts found in the system.\n");
    }
}

void searchAccount(FILE *fPtr) {
    char searchName[15];
    ClientData client = {0, "", "", 0.0, "", ""};
    int found = 0;

    printf("Enter customer Last Name to search: ");
    scanf("%14s", searchName);

    printTableHeader();
    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        // Case-insensitive comparison could be used here, but keeping standard strcmp
        if (client.acctNum != 0 && strcmp(client.lastName, searchName) == 0) {
            displayRecord(&client);
            found = 1;
        }
    }
    printTableFooter();

    if (!found) {
        fprintf(stderr, "No accounts found for last name: %s\n", searchName);
    }
}

void sortAndDisplay(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", ""};
    
    // Dynamic memory allocation for active accounts
    // We only allocate memory for what we actually need
    ClientData *activeClients = NULL;
    int count = 0;

    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            count++;
            // Realloc is used for dynamic array growth
            activeClients = (ClientData *)realloc(activeClients, count * sizeof(ClientData));
            if (activeClients == NULL) {
                fprintf(stderr, "Critical Memory Error: Could not allocate memory.\n");
                return;
            }
            activeClients[count - 1] = client;
        }
    }

    if (count == 0) {
        printf("No active accounts to display.\n");
        return;
    }

    // Advanced Sorting using C Standard Library qsort
    qsort(activeClients, count, sizeof(ClientData), compareByBalance);

    printf("\nSorted Records by Balance (Descending):\n");
    printTableHeader();
    for (int i = 0; i < count; i++) {
        displayRecord(&activeClients[i]);
    }
    printTableFooter();

    // Free dynamic memory to prevent memory leaks
    free(activeClients);
}

void transferBalance(FILE *fPtr) {
    unsigned int srcAcct, destAcct;
    double amount;
    ClientData srcClient = {0, "", "", 0.0, "", ""};
    ClientData destClient = {0, "", "", 0.0, "", ""};

    printf("Enter Source Account (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &srcAcct) != 1 || srcAcct < 1 || srcAcct > MAX_ACCOUNTS) {
        fprintf(stderr, "Invalid source account.\n"); clearInputBuffer(); return;
    }
    fseek(fPtr, (srcAcct - 1) * sizeof(ClientData), SEEK_SET);
    fread(&srcClient, sizeof(ClientData), 1, fPtr);
    if (srcClient.acctNum == 0) {
        fprintf(stderr, "Source account does not exist.\n"); return;
    }

    printf("Enter Destination Account (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &destAcct) != 1 || destAcct < 1 || destAcct > MAX_ACCOUNTS || destAcct == srcAcct) {
        fprintf(stderr, "Invalid destination account.\n"); clearInputBuffer(); return;
    }
    fseek(fPtr, (destAcct - 1) * sizeof(ClientData), SEEK_SET);
    fread(&destClient, sizeof(ClientData), 1, fPtr);
    if (destClient.acctNum == 0) {
        fprintf(stderr, "Destination account does not exist.\n"); return;
    }

    printf("Enter Amount to Transfer: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        fprintf(stderr, "Invalid amount.\n"); clearInputBuffer(); return;
    }

    if (srcClient.balance - amount < 0) {
        fprintf(stderr, "Transfer declined! Insufficient balance in source account.\n"); return;
    }

    // Deduct from source
    srcClient.balance -= amount;
    fseek(fPtr, (srcAcct - 1) * sizeof(ClientData), SEEK_SET);
    fwrite(&srcClient, sizeof(ClientData), 1, fPtr);

    // Add to destination
    destClient.balance += amount;
    fseek(fPtr, (destAcct - 1) * sizeof(ClientData), SEEK_SET);
    fwrite(&destClient, sizeof(ClientData), 1, fPtr);

    printf("Successfully transferred %.2f from Acct %u to Acct %u\n", amount, srcAcct, destAcct);
    logTransaction("Transfer Out", srcAcct, -amount);
    logTransaction("Transfer In", destAcct, amount);
}
// Bank-account program: ULTRA MEGA Architectural Deep Update
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define FILE_NAME "credit_ultra.dat"
#define LOG_FILE "transactions.log"

// --- ANSI COLOR CODES ---
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET "\x1b[0m"

// --- ADVANCED DATA STRUCTURES ---

typedef enum {
    MENU_TEXT_FILE = 1,
    MENU_UPDATE,
    MENU_ADD,
    MENU_DELETE,
    MENU_LIST,
    MENU_SEARCH,
    MENU_SORT,
    MENU_TRANSFER,
    MENU_LOAN,
    MENU_CURRENCY,
    MENU_BACKUP,
    MENU_FREEZE,
    MENU_INTEREST,
    MENU_MINI_STATEMENT,
    MENU_EXIT
} MenuOption;

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
    unsigned int pin;          // 4-digit security PIN
    int isFrozen;              // Boolean flag (1=frozen, 0=active)
    double loanDebt;           // New Field: Tracks active loan amount
    unsigned int creditScore;  // New Field: 300 to 850 scoring system
} ClientData;

// --- UTILITY & VALIDATION FUNCTIONS ---

void clearInputBuffer(void) {
    int c;
    // LOOPS EXPLANATION: This 'while' loop executes repeatedly until it consumes 
    // a newline character or EOF. It is essential for clearing standard input.
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void clearScreen(void) {
    system("cls || clear");
}

void logTransaction(const char* action, unsigned int acctNum, double amount) {
    FILE *logPtr = fopen(LOG_FILE, "a");
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

void maskMobile(char *masked, const char *original) {
    strcpy(masked, "******");
    int len = strlen(original);
    if (len >= 4) {
        strcat(masked, original + len - 4);
    } else {
        strcat(masked, "0000");
    }
}

int verifyPin(const ClientData *client) {
    unsigned int inputPin;
    printf(COLOR_YELLOW "Enter 4-digit PIN for Account %u: " COLOR_RESET, client->acctNum);
    if (scanf("%u", &inputPin) != 1) {
        clearInputBuffer();
        return 0;
    }
    return (inputPin == client->pin);
}

void printTableHeader(void) {
    printf(COLOR_CYAN "\n===================================================================================================================\n");
    printf("| %-6s | %-12s | %-12s | %-10s | %-8s | %-10s | %-10s | %-5s | %-8s |\n", 
           "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance", "Debt", "Score", "Status");
    printf("===================================================================================================================\n" COLOR_RESET);
}

void printTableFooter(void) {
    printf(COLOR_CYAN "===================================================================================================================\n\n" COLOR_RESET);
}

// POINTER EXPLANATION: We pass 'const ClientData *client' as a pointer instead of by value.
// This means we only pass an 8-byte memory address rather than cloning all 60+ bytes of the struct. 
// This vastly improves processing speed and memory usage.
void displayRecord(const ClientData *client) {
    char masked[15];
    maskMobile(masked, client->mobileNumber);
    const char* status = client->isFrozen ? COLOR_RED "FROZEN" COLOR_RESET : COLOR_GREEN "ACTIVE" COLOR_RESET;
    
    // Structure access via pointers uses the -> operator instead of the . operator
    printf("| %-6u | %-12s | %-12s | %-10s | %-8s | %10.2f | %10.2f | %5u | %-17s |\n", 
           client->acctNum, client->lastName, client->firstName, 
           masked, client->accountType, client->balance, client->loanDebt, client->creditScore, status);
}

int compareByBalance(const void *a, const void *b) {
    ClientData *clientA = (ClientData *)a;
    ClientData *clientB = (ClientData *)b;
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
void handleLoan(FILE *fPtr);
void viewForeignCurrency(FILE *fPtr);
void backupDatabase(void);
void freezeAccount(FILE *fPtr);
void applyInterest(FILE *fPtr);
void miniStatement(void);

// --- MAIN ---
int main(int argc, char *argv[])
{
    FILE *cfPtr;
    MenuOption choice;

    if ((cfPtr = fopen(FILE_NAME, "rb+")) == NULL) {
        if ((cfPtr = fopen(FILE_NAME, "wb+")) != NULL) {
            ClientData blankClient = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
            for (int i = 0; i < MAX_ACCOUNTS; i++) {
                fwrite(&blankClient, sizeof(ClientData), 1, cfPtr);
            }
            rewind(cfPtr);
        } else {
            fprintf(stderr, COLOR_RED "%s: Critical Error. File could not be opened.\n" COLOR_RESET, argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    clearScreen();
    printf(COLOR_GREEN "--- Welcome to the ULTRA MEGA Banking System ---\n" COLOR_RESET);

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
            case MENU_LOAN: handleLoan(cfPtr); break;
            case MENU_CURRENCY: viewForeignCurrency(cfPtr); break;
            case MENU_BACKUP: backupDatabase(); break;
            case MENU_FREEZE: freezeAccount(cfPtr); break;
            case MENU_INTEREST: applyInterest(cfPtr); break;
            case MENU_MINI_STATEMENT: miniStatement(); break;
            default: fprintf(stderr, COLOR_RED "Incorrect choice. Please try again.\n" COLOR_RESET); break;
        }
    }

    printf(COLOR_GREEN "Exiting system. Have a great day!\n" COLOR_RESET);
    fclose(cfPtr);
    return EXIT_SUCCESS;
}

// --- IMPLEMENTATIONS ---

MenuOption enterChoice(void) {
    unsigned int menuChoice;
    printf(COLOR_CYAN "\n============================================\n");
    printf("             ULTRA MAIN MENU                \n");
    printf("============================================\n" COLOR_RESET);
    printf("1 - Export Accounts to Text File\n");
    printf("2 - Update an Account Balance\n");
    printf("3 - Add a New Account\n");
    printf("4 - Delete an Account\n");
    printf("5 - List All Active Accounts\n");
    printf("6 - Search Account by Last Name\n");
    printf("7 - Sort and Display by Balance\n");
    printf("8 - Transfer Balance\n");
    printf(COLOR_MAGENTA "9 - Request or Pay Loan\n" COLOR_RESET);
    printf(COLOR_MAGENTA "10- Multi-Currency Balance Viewer\n" COLOR_RESET);
    printf(COLOR_YELLOW "11- Admin: Backup Database\n" COLOR_RESET);
    printf(COLOR_YELLOW "12- Admin: Freeze/Unfreeze Account\n" COLOR_RESET);
    printf(COLOR_YELLOW "13- Admin: Apply Monthly Interest\n" COLOR_RESET);
    printf("14- Generate Mini-Statement\n");
    printf("15- End Program\n");
    printf(COLOR_CYAN "============================================\n" COLOR_RESET);
    printf("Enter choice: ");

    if (scanf("%u", &menuChoice) != 1) {
        clearInputBuffer();
        return (MenuOption)0;
    }
    return (MenuOption)menuChoice;
}

void textFile(FILE *readPtr) {
    FILE *writePtr;
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) {
        fprintf(stderr, COLOR_RED "Error: Could not open accounts.txt for writing.\n" COLOR_RESET);
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s %-12s %-12s %-10s %-8s %10s %10s %5s %-8s\n", 
            "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance", "Debt", "Score", "Status");
    fprintf(writePtr, "-------------------------------------------------------------------------------------------------\n");

    while (fread(&client, sizeof(ClientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            char masked[15];
            maskMobile(masked, client.mobileNumber);
            fprintf(writePtr, "%-6u %-12s %-12s %-10s %-8s %10.2f %10.2f %5u %-8s\n", 
                    client.acctNum, client.lastName, client.firstName, 
                    masked, client.accountType, client.balance, client.loanDebt, 
                    client.creditScore, client.isFrozen ? "FROZEN" : "ACTIVE");
        }
    }
    fclose(writePtr);
    printf(COLOR_GREEN "Success: Exported active records to 'accounts.txt' (Privacy Masked).\n" COLOR_RESET);
}

void updateRecord(FILE *fPtr) {
    unsigned int account;
    double transaction;
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};

    printf("Enter account to update (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid account number.\n" COLOR_RESET);
        clearInputBuffer(); return;
    }

    fseek(fPtr, (account - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        fprintf(stderr, COLOR_RED "Account #%u has no information.\n" COLOR_RESET, account); return;
    }
    if (client.isFrozen) {
        fprintf(stderr, COLOR_RED "Account is FROZEN! Transactions blocked. Contact Admin.\n" COLOR_RESET); return;
    }
    if (!verifyPin(&client)) {
        fprintf(stderr, COLOR_RED "Security Alert: Invalid PIN. Access Denied.\n" COLOR_RESET); return;
    }

    printTableHeader();
    displayRecord(&client);
    printTableFooter();

    printf("Enter charge (-) or payment (+): ");
    if (scanf("%lf", &transaction) != 1) {
        fprintf(stderr, COLOR_RED "Invalid transaction amount.\n" COLOR_RESET);
        clearInputBuffer(); return;
    }
    
    // CONDITIONAL EXPLANATION: The if-else block directs program flow based on boolean logic. 
    // Here it serves as the Negative Balance Restriction safeguard.
    if (client.balance + transaction < 0) {
        fprintf(stderr, COLOR_RED "Transaction declined! Insufficient balance.\n" COLOR_RESET);
        client.creditScore = (client.creditScore > 5) ? client.creditScore - 5 : 300; // Penalty
    } else {
        client.balance += transaction;
        printf(COLOR_GREEN "Transaction successful. New balance: %.2f\n" COLOR_RESET, client.balance);
        logTransaction(transaction >= 0 ? "Deposit" : "Withdrawal", client.acctNum, transaction);
    }

    // Save changes
    fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
    fwrite(&client, sizeof(ClientData), 1, fPtr);
}

void handleLoan(FILE *fPtr) {
    unsigned int account;
    int opt;
    double amount;
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};

    printf("Enter account number (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid account.\n" COLOR_RESET); clearInputBuffer(); return;
    }
    fseek(fPtr, (account - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0 || client.isFrozen) {
        fprintf(stderr, COLOR_RED "Account unavailable or frozen.\n" COLOR_RESET); return;
    }
    if (!verifyPin(&client)) {
        fprintf(stderr, COLOR_RED "Security Alert: Invalid PIN. Access Denied.\n" COLOR_RESET); return;
    }

    printf(COLOR_CYAN "\n--- LOAN CENTER ---\n" COLOR_RESET);
    printf("1. Request New Loan\n");
    printf("2. Pay Off Existing Loan\n");
    printf("Choice: ");
    if (scanf("%d", &opt) != 1) { clearInputBuffer(); return; }

    if (opt == 1) {
        printf("Current Credit Score: %u\n", client.creditScore);
        if (client.creditScore < 500) {
            fprintf(stderr, COLOR_RED "Loan Denied: Credit score too low.\n" COLOR_RESET); return;
        }
        double maxLoan = client.balance * 3.0; // Dynamic max loan calculation
        printf("Max Approved Loan Amount: %.2f\nEnter request amount: ", maxLoan);
        if (scanf("%lf", &amount) != 1 || amount <= 0 || amount > maxLoan) {
            fprintf(stderr, COLOR_RED "Loan Denied: Invalid amount or exceeds limit.\n" COLOR_RESET); clearInputBuffer(); return;
        }
        client.loanDebt += amount;
        client.balance += amount;
        client.creditScore = (client.creditScore > 20) ? client.creditScore - 20 : 300; // Small hit for taking debt
        printf(COLOR_GREEN "Loan Approved! %.2f deposited. Credit score adjusted.\n" COLOR_RESET, amount);
        logTransaction("Loan Dispersed", client.acctNum, amount);

    } else if (opt == 2) {
        if (client.loanDebt <= 0) {
            printf(COLOR_YELLOW "You have no active loans to pay off.\n" COLOR_RESET); return;
        }
        printf("Current Debt: %.2f\nEnter payment amount: ", client.loanDebt);
        if (scanf("%lf", &amount) != 1 || amount <= 0 || amount > client.balance) {
            fprintf(stderr, COLOR_RED "Invalid amount or insufficient balance.\n" COLOR_RESET); clearInputBuffer(); return;
        }
        double payAmt = (amount > client.loanDebt) ? client.loanDebt : amount;
        client.balance -= payAmt;
        client.loanDebt -= payAmt;
        client.creditScore = (client.creditScore < 840) ? client.creditScore + 10 : 850; // Boost for paying debt
        printf(COLOR_GREEN "Payment successful! %.2f paid. Credit score improved.\n" COLOR_RESET, payAmt);
        logTransaction("Loan Payment", client.acctNum, -payAmt);
    } else {
        printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
    }

    fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
    fwrite(&client, sizeof(ClientData), 1, fPtr);
}

void viewForeignCurrency(FILE *fPtr) {
    unsigned int account;
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    
    printf("Enter account number (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        clearInputBuffer(); return;
    }
    fseek(fPtr, (account - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0 || !verifyPin(&client)) return;

    double eur = client.balance * 0.93;
    double gbp = client.balance * 0.79;
    double jpy = client.balance * 155.40;

    printf(COLOR_MAGENTA "\n--- GLOBAL CURRENCY CONVERTER ---\n" COLOR_RESET);
    printf("Base Balance (USD): $ %.2f\n", client.balance);
    printf("Euro (EUR):         € %.2f\n", eur);
    printf("British Pound (GBP):£ %.2f\n", gbp);
    printf("Japanese Yen (JPY): ¥ %.2f\n", jpy);
    printf(COLOR_MAGENTA "---------------------------------\n" COLOR_RESET);
}

void backupDatabase(void) {
    FILE *src = fopen(FILE_NAME, "rb");
    FILE *dest = fopen("backup.dat", "wb");
    if (!src || !dest) {
        fprintf(stderr, COLOR_RED "Backup Failed: File I/O Error.\n" COLOR_RESET);
        if (src) fclose(src);
        if (dest) fclose(dest);
        return;
    }
    
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }
    
    fclose(src);
    fclose(dest);
    printf(COLOR_GREEN "SYSTEM BACKUP COMPLETE: Database cloned to backup.dat\n" COLOR_RESET);
}

void transferBalance(FILE *fPtr) {
    unsigned int srcAcct, destAcct;
    double amount;
    ClientData srcClient = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    ClientData destClient = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};

    printf("Enter Source Account (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &srcAcct) != 1 || srcAcct < 1 || srcAcct > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid source account.\n" COLOR_RESET); clearInputBuffer(); return;
    }
    fseek(fPtr, (srcAcct - 1) * sizeof(ClientData), SEEK_SET);
    fread(&srcClient, sizeof(ClientData), 1, fPtr);
    if (srcClient.acctNum == 0) {
        fprintf(stderr, COLOR_RED "Source account does not exist.\n" COLOR_RESET); return;
    }
    if (srcClient.isFrozen) {
        fprintf(stderr, COLOR_RED "Source account is FROZEN! Transfers blocked.\n" COLOR_RESET); return;
    }
    if (!verifyPin(&srcClient)) {
        fprintf(stderr, COLOR_RED "Security Alert: Invalid PIN. Access Denied.\n" COLOR_RESET); return;
    }

    printf("Enter Destination Account (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &destAcct) != 1 || destAcct < 1 || destAcct > MAX_ACCOUNTS || destAcct == srcAcct) {
        fprintf(stderr, COLOR_RED "Invalid destination account.\n" COLOR_RESET); clearInputBuffer(); return;
    }
    fseek(fPtr, (destAcct - 1) * sizeof(ClientData), SEEK_SET);
    fread(&destClient, sizeof(ClientData), 1, fPtr);
    if (destClient.acctNum == 0) {
        fprintf(stderr, COLOR_RED "Destination account does not exist.\n" COLOR_RESET); return;
    }
    if (destClient.isFrozen) {
        fprintf(stderr, COLOR_RED "Destination account is FROZEN! Cannot receive funds.\n" COLOR_RESET); return;
    }

    printf("Enter Amount to Transfer: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        fprintf(stderr, COLOR_RED "Invalid amount.\n" COLOR_RESET); clearInputBuffer(); return;
    }
    if (srcClient.balance - amount < 0) {
        fprintf(stderr, COLOR_RED "Transfer declined! Insufficient balance.\n" COLOR_RESET); return;
    }

    srcClient.balance -= amount;
    fseek(fPtr, (srcAcct - 1) * sizeof(ClientData), SEEK_SET);
    fwrite(&srcClient, sizeof(ClientData), 1, fPtr);

    destClient.balance += amount;
    fseek(fPtr, (destAcct - 1) * sizeof(ClientData), SEEK_SET);
    fwrite(&destClient, sizeof(ClientData), 1, fPtr);

    printf(COLOR_GREEN "Successfully transferred %.2f from Acct %u to Acct %u\n" COLOR_RESET, amount, srcAcct, destAcct);
    logTransaction("Transfer Out", srcAcct, -amount);
    logTransaction("Transfer In", destAcct, amount);
}

void deleteRecord(FILE *fPtr) {
    ClientData client;
    ClientData blankClient = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    unsigned int accountNum;

    printf("Enter account number to delete (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid input.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        fprintf(stderr, COLOR_RED "Account %u does not exist.\n" COLOR_RESET, accountNum);
    } else {
        if (!verifyPin(&client)) {
            fprintf(stderr, COLOR_RED "Security Alert: Invalid PIN. Deletion Denied.\n" COLOR_RESET); return;
        }
        if (client.loanDebt > 0) {
            fprintf(stderr, COLOR_RED "Cannot delete! Account has outstanding debt: %.2f\n" COLOR_RESET, client.loanDebt); return;
        }
        fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
        fwrite(&blankClient, sizeof(ClientData), 1, fPtr);
        printf(COLOR_GREEN "Account %u successfully deleted.\n" COLOR_RESET, accountNum);
        
        logTransaction("Account Deleted", accountNum, 0.0);
    }
}

void newRecord(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    unsigned int accountNum;

    printf("Enter new account number (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid input.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum != 0) {
        fprintf(stderr, COLOR_RED "Account #%u already contains information.\n" COLOR_RESET, client.acctNum); return;
    }

    printf("Enter First Name: ");
    scanf("%9s", client.firstName);
    if (!isValidName(client.firstName)) {
        fprintf(stderr, COLOR_RED "Invalid Name.\n" COLOR_RESET); return;
    }

    printf("Enter Last Name: ");
    scanf("%14s", client.lastName);
    if (!isValidName(client.lastName)) {
        fprintf(stderr, COLOR_RED "Invalid Name.\n" COLOR_RESET); return;
    }

    printf("Enter Mobile Number (10 digits): ");
    scanf("%14s", client.mobileNumber);
    if (!isValidMobile(client.mobileNumber)) {
        fprintf(stderr, COLOR_RED "Invalid Mobile.\n" COLOR_RESET); return;
    }

    printf("Enter Account Type (Savings/Current): ");
    scanf("%9s", client.accountType);

    printf("Set a 4-digit PIN: ");
    if (scanf("%u", &client.pin) != 1 || client.pin < 1000 || client.pin > 9999) {
        fprintf(stderr, COLOR_RED "Invalid PIN. Must be 4 digits.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    printf("Enter Initial Balance: ");
    if (scanf("%lf", &client.balance) != 1 || client.balance < 0) {
        fprintf(stderr, COLOR_RED "Invalid Balance.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    client.acctNum = accountNum;
    client.isFrozen = 0; 
    client.loanDebt = 0.0;
    client.creditScore = 600; // Base credit score

    fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
    fwrite(&client, sizeof(ClientData), 1, fPtr);
    printf(COLOR_GREEN "Success: Account %u created securely.\n" COLOR_RESET, accountNum);
    
    logTransaction("Account Created", accountNum, client.balance);
}

void listAllRecords(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
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

    if (!found) printf(COLOR_YELLOW "No active accounts found in the system.\n" COLOR_RESET);
}

void searchAccount(FILE *fPtr) {
    char searchName[15];
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    int found = 0;

    printf("Enter customer Last Name to search: ");
    scanf("%14s", searchName);

    printTableHeader();
    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum != 0 && strcmp(client.lastName, searchName) == 0) {
            displayRecord(&client);
            found = 1;
        }
    }
    printTableFooter();

    if (!found) fprintf(stderr, COLOR_YELLOW "No accounts found for last name: %s\n" COLOR_RESET, searchName);
}

void sortAndDisplay(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    ClientData *activeClients = NULL;
    int count = 0;

    rewind(fPtr);
    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            count++;
            activeClients = (ClientData *)realloc(activeClients, count * sizeof(ClientData));
            if (activeClients == NULL) {
                fprintf(stderr, COLOR_RED "Critical Memory Error: Could not allocate memory.\n" COLOR_RESET);
                return;
            }
            activeClients[count - 1] = client;
        }
    }

    if (count == 0) {
        printf(COLOR_YELLOW "No active accounts to display.\n" COLOR_RESET);
        return;
    }

    qsort(activeClients, count, sizeof(ClientData), compareByBalance);

    printf(COLOR_GREEN "\nSorted Records by Balance (Descending):\n" COLOR_RESET);
    printTableHeader();
    for (int i = 0; i < count; i++) {
        displayRecord(&activeClients[i]);
    }
    printTableFooter();

    free(activeClients);
}

void freezeAccount(FILE *fPtr) {
    unsigned int account;
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};

    printf(COLOR_YELLOW "--- ADMIN CONSOLE ---\n" COLOR_RESET);
    printf("Enter account to toggle Freeze Status (1 - %d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        fprintf(stderr, COLOR_RED "Invalid input.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    fseek(fPtr, (account - 1) * sizeof(ClientData), SEEK_SET);
    fread(&client, sizeof(ClientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        fprintf(stderr, COLOR_RED "Account #%u has no information.\n" COLOR_RESET, account); return;
    }

    client.isFrozen = !client.isFrozen;
    
    fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
    fwrite(&client, sizeof(ClientData), 1, fPtr);
    
    printf(COLOR_GREEN "Account %u is now %s.\n" COLOR_RESET, account, client.isFrozen ? "FROZEN" : "ACTIVE");
    logTransaction(client.isFrozen ? "Account Frozen" : "Account Unfrozen", account, 0.0);
}

void applyInterest(FILE *fPtr) {
    ClientData client = {0, "", "", 0.0, "", "", 0, 0, 0.0, 0};
    int count = 0;
    double interestRate = 0.02; // 2% monthly interest

    printf(COLOR_YELLOW "--- ADMIN CONSOLE: Apply Interest ---\n" COLOR_RESET);
    printf("Applying 2%% interest to all ACTIVE 'Savings' accounts...\n");

    rewind(fPtr);
    for (int i = 1; i <= MAX_ACCOUNTS; i++) {
        fseek(fPtr, (i - 1) * sizeof(ClientData), SEEK_SET);
        fread(&client, sizeof(ClientData), 1, fPtr);

        if (client.acctNum != 0 && !client.isFrozen && strcmp(client.accountType, "Savings") == 0) {
            double interestAmount = client.balance * interestRate;
            client.balance += interestAmount;
            
            fseek(fPtr, - (long) sizeof(ClientData), SEEK_CUR);
            fwrite(&client, sizeof(ClientData), 1, fPtr);
            
            logTransaction("Interest Applied", client.acctNum, interestAmount);
            count++;
        }
    }
    printf(COLOR_GREEN "Successfully applied interest to %d active Savings accounts.\n" COLOR_RESET, count);
}

void miniStatement(void) {
    unsigned int searchAcct;
    char buffer[256];
    int found = 0;

    printf("Enter Account Number for Mini-Statement: ");
    if (scanf("%u", &searchAcct) != 1) {
        fprintf(stderr, COLOR_RED "Invalid input.\n" COLOR_RESET); clearInputBuffer(); return;
    }

    FILE *logPtr = fopen(LOG_FILE, "r");
    if (logPtr == NULL) {
        fprintf(stderr, COLOR_RED "Error: No transaction log found.\n" COLOR_RESET); return;
    }

    printf(COLOR_CYAN "\n=== MINI STATEMENT FOR ACCOUNT %u ===\n" COLOR_RESET, searchAcct);
    char targetStr[32];
    sprintf(targetStr, "Acct: %-6u", searchAcct);

    while (fgets(buffer, sizeof(buffer), logPtr) != NULL) {
        if (strstr(buffer, targetStr) != NULL) {
            printf("%s", buffer);
            found = 1;
        }
    }
    
    if (!found) printf(COLOR_YELLOW "No transactions found for this account.\n" COLOR_RESET);
    printf(COLOR_CYAN "=======================================\n" COLOR_RESET);
    fclose(logPtr);
}
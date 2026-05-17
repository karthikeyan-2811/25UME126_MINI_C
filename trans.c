// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100

// helper to clear input buffer
// LOOPS EXPLANATION: This 'while' loop repeatedly executes getchar() until it 
// hits a newline or End of File. It is used to consume invalid characters.
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// clientData structure definition
// VARIABLES EXPLANATION: A struct is a composite variable that groups multiple 
// different variables (like unsigned int, char array, double) into one logical unit.
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
    char mobileNumber[15]; // mobile number
    char accountType[10];  // account type (Savings/Current)
};

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAllRecords(FILE *fPtr);

// New features prototypes
void searchAccount(FILE *fPtr);
void sortAndDisplay(FILE *fPtr);
void displayRecord(const struct clientData *client);
void processTransaction(struct clientData *client, double amount);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit_v2.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    // using credit_v2.dat because the structure size changed
    if ((cfPtr = fopen("credit_v2.dat", "rb+")) == NULL)
    {
        // attempt to create file if it doesn't exist
        if ((cfPtr = fopen("credit_v2.dat", "wb+")) != NULL) {
            struct clientData blankClient = {0, "", "", 0.0, "", ""};
            for (int i = 0; i < MAX_ACCOUNTS; i++) {
                fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
            }
            rewind(cfPtr);
        } else {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(-1);
        }
    }

    // enable user to specify action
    // LOOPS EXPLANATION: This while loop acts as the main program loop. It keeps
    // calling enterChoice() until the user returns 8.
    while ((choice = enterChoice()) != 8)
    {
        // SWITCH-CASE EXPLANATION: The switch statement provides a clean way to 
        // branch execution based on the integer value of 'choice'. It is much more
        // readable than a long chain of if-else statements.
        switch (choice)
        {
        case 1: textFile(cfPtr); break;
        case 2: updateRecord(cfPtr); break;
        case 3: newRecord(cfPtr); break;
        case 4: deleteRecord(cfPtr); break;
        case 5: listAllRecords(cfPtr); break;
        case 6: searchAccount(cfPtr); break;
        case 7: sortAndDisplay(cfPtr); break;
        default: puts("Incorrect choice"); break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    struct clientData client = {0, "", "", 0.0, "", ""};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    }
    else
    {
        rewind(readPtr);
        fprintf(writePtr, "%-6s%-16s%-11s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");

        while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
        {
            if (client.acctNum != 0)
            {
                fprintf(writePtr, "%-6u%-16s%-11s%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.mobileNumber, client.accountType, client.balance);
            }
        }
        fclose(writePtr);
        puts("accounts.txt successfully generated.");
    }
}

// Display a single record
// POINTER EXPLANATION: Passing a 'const struct clientData *' pointer is highly efficient. 
// Instead of copying all 50+ bytes of the struct onto the stack, we just pass an 8-byte 
// memory address. The 'const' keyword ensures the display function cannot accidentally modify it.
void displayRecord(const struct clientData *client) {
    printf("%-6u%-16s%-11s%-16s%-11s%10.2f\n", client->acctNum, client->lastName, client->firstName, client->mobileNumber, client->accountType, client->balance);
}

// Process a transaction using pointers, with negative balance restriction
// CONDITIONAL EXPLANATION: The 'if/else' block here checks a condition before modifying 
// data, acting as a crucial banking safeguard (Negative Balance Restriction).
void processTransaction(struct clientData *client, double amount) {
    if (client->balance + amount < 0) {
        printf("Transaction declined! Insufficient balance. Current Balance: %.2f\n", client->balance);
    } else {
        client->balance += amount; // Pointer updates balance directly in memory
        printf("Transaction successful. New balance: %.2f\n", client->balance);
    }
}

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0, "", ""};

    printf("Enter account to update ( 1 - %d ): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1) {
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return;
    }

    if (account < 1 || account > MAX_ACCOUNTS) {
        printf("Invalid account number. It must be between 1 and %d.\n", MAX_ACCOUNTS);
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    {
        printf("\n%-6s%-16s%-11s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
        displayRecord(&client);

        printf("\nEnter charge ( - ) or payment ( + ): ");
        if (scanf("%lf", &transaction) != 1) {
            printf("Invalid transaction amount.\n");
            clearInputBuffer();
            return;
        }
        
        processTransaction(&client, transaction);

        fseek(fPtr, - (long) sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter account number to delete ( 1 - %d ): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1) {
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return;
    }

    if (accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        printf("Invalid account number. It must be between 1 and %d.\n", MAX_ACCOUNTS);
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    }
    else
    {
        fseek(fPtr, - (long) sizeof(struct clientData), SEEK_CUR);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        printf("Account %u successfully deleted.\n", accountNum);
    }
}

// create and insert record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0, "", ""};
    unsigned int accountNum;

    printf("Enter new account number ( 1 - %d ): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1) {
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return;
    }

    if (accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        printf("Invalid account number. It must be between 1 and %d.\n", MAX_ACCOUNTS);
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    }
    else
    {
        printf("Enter lastname, firstname, mobile, type (Savings/Current), balance\n? ");
        if (scanf("%14s%9s%14s%9s%lf", client.lastName, client.firstName, client.mobileNumber, client.accountType, &client.balance) != 5) {
            printf("Invalid input. Account creation aborted.\n");
            clearInputBuffer();
            return;
        }

        client.acctNum = accountNum;
        fseek(fPtr, - (long) sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        printf("Account %u successfully created.\n", accountNum);
    }
}

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice;
    printf("\n%s", "Enter your choice\n"
                 "1 - store a formatted text file of accounts called \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - search account by customer last name\n"
                 "7 - display sorted records by balance\n"
                 "8 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1) {
        clearInputBuffer();
        return 0; // return invalid choice
    }
    return menuChoice;
}

// list all active records to console
void listAllRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0, "", ""};

    printf("\n%-6s%-16s%-11s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
    
    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            displayRecord(&client);
        }
    }
    printf("\n");
}

// search account by customer name
void searchAccount(FILE *fPtr) {
    char searchName[15];
    struct clientData client = {0, "", "", 0.0, "", ""};
    int found = 0;

    printf("Enter customer last name to search: ");
    scanf("%14s", searchName);

    printf("\n%-6s%-16s%-11s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
    
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum != 0 && strcmp(client.lastName, searchName) == 0) {
            displayRecord(&client);
            found = 1;
        }
    }

    if (!found) {
        printf("No accounts found for last name: %s\n", searchName);
    }
}

// sort and display records by balance
void sortAndDisplay(FILE *fPtr) {
    struct clientData clients[MAX_ACCOUNTS];
    struct clientData temp;
    int count = 0;
    struct clientData client = {0, "", "", 0.0, "", ""};

    // Load active records into array
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            clients[count++] = client;
        }
    }

    if (count == 0) {
        printf("No active accounts to display.\n");
        return;
    }

    // Bubble Sort by balance (descending)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (clients[j].balance < clients[j+1].balance) {
                temp = clients[j];
                clients[j] = clients[j+1];
                clients[j+1] = temp;
            }
        }
    }

    // Display sorted array
    printf("\nSorted Records by Balance (Descending):\n");
    printf("\n%-6s%-16s%-11s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Mobile", "Type", "Balance");
    for (int i = 0; i < count; i++) {
        displayRecord(&clients[i]);
    }
}
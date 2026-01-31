// Bank Account Management System
// This program manages bank accounts using a random-access file (credit.dat).
// It allows creating text files, updating records, adding new records, deleting records,
// withdrawing funds, listing accounts, and searching for accounts.
// Enhancements include:
// - Security: Simple password protection to access the system.
// - Withdrawal Details: Separate withdrawal function with balance checks and transaction logging.
// - Input Validation: Robust checks for invalid inputs.
// - Confirmation Prompts: For deletions and withdrawals.
// - Transaction Logging: Logs all transactions to a file (transactions.log).
// - Additional Features: List all accounts, search by account number, apply interest.
// - Error Handling: Improved error messages and file handling.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // For input validation and tolower

#define MAX_ACCOUNTS 100
#define INTEREST_RATE 5.0  // 5% annual interest
#define PASSWORD "saran1973"  // Simple password for security

// clientData structure definition
struct clientData {
    unsigned int acctNum;  // account number
    char lastName[15];     // account last name
    char firstName[10];    // account first name
    double balance;        // account balance
};  // end structure clientData

// Prototypes
int authenticate(void);  // Password authentication
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void withdrawRecord(FILE *fPtr);  // New: Withdrawal function
void listAccounts(FILE *fPtr);    // New: List all accounts
void searchAccount(FILE *fPtr);   // New: Search account
void applyInterest(FILE *fPtr);   // New: Apply interest
void logTransaction(unsigned int acctNum, const char *type, double amount, double newBalance);  // Log transactions
void clearInputBuffer(void);  // Helper for input validation

int main(int argc, char *argv[]) {
    FILE *cfPtr;          // credit.dat file pointer
    unsigned int choice;  // user's choice

    // Authenticate user
    if (!authenticate()) {
        printf("Authentication failed. Exiting...\n");
        return 1;
    }

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // Initialize file if empty (ensure 100 blank records)
    fseek(cfPtr, 0, SEEK_END);
    if (ftell(cfPtr) == 0) {
        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < MAX_ACCOUNTS; i++) {
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
        }
    }
    rewind(cfPtr);

    // Enable user to specify action
    while ((choice = enterChoice()) != 9) {
        switch (choice) {
            case 1: textFile(cfPtr); break;
            case 2: updateRecord(cfPtr); break;
            case 3: newRecord(cfPtr); break;
            case 4: deleteRecord(cfPtr); break;
            case 5: withdrawRecord(cfPtr); break;  // New option
            case 6: listAccounts(cfPtr); break;     // New option
            case 7: searchAccount(cfPtr); break;    // New option
            case 8: applyInterest(cfPtr); break;    // New option
            case 9: break;  // Exit
            default: puts("Incorrect choice. Please select 1-9."); break;
        }
    }

    fclose(cfPtr);  // fclose closes the file
    return 0;
}

// Password authentication function
int authenticate(void) {
    char input[20];
    printf("Enter password: ");
    scanf("%19s", input);
    if (strcmp(input, PASSWORD) == 0) {
        return 1;  // Success
    } else {
        return 0;  // Failure
    }
}

// Enable user to input menu choice
unsigned int enterChoice(void) {
    unsigned int menuChoice;
    printf("\n%s", "Enter your choice\n"
                   "1 - Store a formatted text file of accounts called \"accounts.txt\" for printing\n"
                   "2 - Update an account (deposit/payment)\n"
                   "3 - Add a new account\n"
                   "4 - Delete an account\n"
                   "5 - Withdraw from an account\n"
                   "6 - List all accounts\n"
                   "7 - Search an account\n"
                   "8 - Apply interest to all accounts\n"
                   "9 - End program\n? ");
    while (scanf("%u", &menuChoice) != 1) {
        clearInputBuffer();
        printf("Invalid input. Enter your choice (1-9): ");
    }
    return menuChoice;
}

// Helper to clear input buffer
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Create formatted text file for printing
void textFile(FILE *readPtr) {
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) {
        puts("File could not be opened.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
        }
    }

    fclose(writePtr);
    puts("accounts.txt created successfully.");
}

// Update balance in record (deposit or payment)
void updateRecord(FILE *fPtr) {
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};
    char confirm;

    printf("Enter account to update (1-100): ");
    while (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        clearInputBuffer();
        printf("Invalid account number. Enter 1-100: ");
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account #%d has no information.\n", account);
        return;
    }

    printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);
    printf("Enter charge (+) or payment (-): ");
    while (scanf("%lf", &transaction) != 1) {
        clearInputBuffer();
        printf("Invalid amount. Enter charge (+) or payment (-): ");
    }

    printf("Confirm transaction of %.2f (y/n)? ", transaction);
    clearInputBuffer();
    scanf("%c", &confirm);
    if (tolower(confirm) != 'y') {
        puts("Transaction cancelled.");
        return;
    }

    client.balance += transaction;
    fseek(fPtr, -((long)sizeof(struct clientData)), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Updated: %-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
    logTransaction(account, transaction > 0 ? "Deposit" : "Payment", transaction, client.balance);
}

// Delete an existing record
void deleteRecord(FILE *fPtr) {
    struct clientData client, blankClient = {0, "", "", 0};
    unsigned int accountNum;
    char confirm;

    printf("Enter account number to delete (1-100): ");
    while (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        clearInputBuffer();
        printf("Invalid account number. Enter 1-100: ");
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account %d does not exist.\n", accountNum);
        return;
    }

    printf("Confirm deletion of account %d (y/n)? ", accountNum);
    clearInputBuffer();
    scanf("%c", &confirm);
    if (tolower(confirm) != 'y') {
        puts("Deletion cancelled.");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    puts("Account deleted.");
    logTransaction(accountNum, "Deletion", 0, 0);
}

// Create and insert record
void newRecord(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter new account number (1-100): ");
    while (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        clearInputBuffer();
        printf("Invalid account number. Enter 1-100: ");
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) {
        printf("Account #%d already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter lastname, firstname, balance\n? ");
    while (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3) {
        clearInputBuffer();
        printf("Invalid input. Enter lastname, firstname, balance: ");
    }

    client.acctNum = accountNum;
    fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    puts("Account created.");
    logTransaction(accountNum, "Creation", 0, client.balance);
}

// New: Withdraw from an account
void withdrawRecord(FILE *fPtr) {
    unsigned int account;
    double amount;
    struct clientData client = {0, "", "", 0.0};
    char confirm;

    printf("Enter account to withdraw from (1-100): ");
    while (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        clearInputBuffer();
        printf("Invalid account number. Enter 1-100: ");
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account #%d has no information.\n", account);
        return;
    }

    printf("Current balance: %.2f\n", client.balance);
    printf("Enter withdrawal amount: ");
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        clearInputBuffer();
        printf("Invalid amount. Enter positive withdrawal amount: ");
    }

    if (amount > client.balance) {
        puts("Insufficient funds.");
        return;
    }

    printf("Confirm withdrawal of %.2f (y/n)? ", amount);
    clearInputBuffer();
    scanf("%c", &confirm);
    if (tolower(confirm) != 'y') {
        puts("Withdrawal cancelled.");
        return;
    }

    client.balance -= amount;
    fseek(fPtr, -((long)sizeof(struct clientData)), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Withdrawal successful. New balance: %.2f\n", client.balance);
    logTransaction(account, "Withdrawal", -amount, client.balance);
}

// New: List all accounts
void listAccounts(FILE *fPtr) {
    struct clientData client;
    int count = 0;

    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            count++;
        }
    }

    printf("\nTotal accounts: %d\n", count);
}

// New: Search an account
void searchAccount(FILE *fPtr) {
    unsigned int account;
    struct clientData client = {0, "", "", 0.0};

    printf("Enter account number to search (1-100): ");
    while (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        clearInputBuffer();
        printf("Invalid account number. Enter 1-100: ");
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account #%d not found.\n", account);
    } else {
        printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
    }
}

// New: Apply interest to all accounts
void applyInterest(FILE *fPtr) {
    struct clientData client;
    int count = 0;

    printf("\nApplying %.1f%% interest...\n", INTEREST_RATE);
    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum != 0) {
            double interest = client.balance * INTEREST_RATE / 100.0;
            client.balance += interest;
            fseek(fPtr, -((long)sizeof(struct clientData)), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
            printf("Account %d: +%.2f (New Balance: %.2f)\n", client.acctNum, interest, client.balance);
            logTransaction(client.acctNum, "Interest", interest, client.balance);
            count++;
        }
    }

    printf("Interest applied to %d accounts.\n", count);
}

// Log transactions to a file
void logTransaction(unsigned int acctNum, const char *type, double amount, double newBalance) {
    FILE *logPtr = fopen("transactions.log", "a");
    if (logPtr) {
        fprintf(logPtr, "Account %d: %s %.2f, New Balance: %.2f\n", acctNum, type, amount, newBalance);
        fclose(logPtr);
    }
}
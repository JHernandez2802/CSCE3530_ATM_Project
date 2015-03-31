// compile with: gcc server.c -l sqlite3 -pthread -o s    add -w to ignore warnings
// Multiple server-client connection; two way communication
	
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>    // for SQLite
#include <sys/types.h>	// system type defintions
#include <sys/socket.h>	// network system functions
#include <netinet/in.h>	// protocol & struct definitions 
#include <pthread.h>    // threading

#define BUF_SIZE	1024
#define LISTEN_PORT	60000

typedef struct transactions
{
	char description[20];
	char amount[11];
};

typedef struct userAccount
{
	char fName[21];
	char lName[21];
	char pin[5];
	char dl[9];
	char ssn[10];
	char email[41];
	int balance;
	int numTransactions;
	struct transactions userTransactions[5];
};

int threadCount = 0;
struct userAccount account;


void *client_handler(void *arg);
static int getRecord(void *NotUsed, int argc, char **argv, char **azColName);
static int getTransactions(void *NotUsed, int argc, char **argv, char **azColName);
int convertStrToInt(char msg[BUF_SIZE], int size);
int findSpace(char msg[BUF_SIZE]);
int power(int num, int pow);

int main(int argc, char *argv[])
{
	int status, *sock_tmp;
	pthread_t a_thread;
    void *thread_result;

    int	sock_listen, sock_recv;
    struct sockaddr_in	my_addr, recv_addr;
    int	i, addr_size, bytes_received, bytes_sent;
    fd_set readfds;
    struct timeval timeout={0,0};
    int	incoming_len;
    struct sockaddr	remote_addr;
    int	recv_msg_size;
    char buf[BUF_SIZE];
    int	select_ret;

    // create socket for listening
    sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_listen < 0)
	{
        printf("socket() failed\n");
        exit(0);
    }

	// make local address structure
    memset(&my_addr, 0, sizeof (my_addr));	// zero out structure
    my_addr.sin_family = AF_INET;	// address family
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // current machine IP
    my_addr.sin_port = htons((unsigned short)LISTEN_PORT);
	
    // bind socket to the local address
    i = bind(sock_listen, (struct sockaddr *) &my_addr, sizeof (my_addr));
    if (i < 0)
	{
        printf("bind() failed\n");
		close(sock_listen);
        exit(0);
    }
	
	// listen ...
    i = listen(sock_listen, 5);
    if (i < 0)
	{
        printf("listen() failed\n");
        exit(0);
    }

    addr_size = sizeof(recv_addr);
	while (1)
	{
		sleep(1);
	
		// Get new socket to receive data on
		sock_recv = accept(sock_listen, (struct sockaddr *) &recv_addr, &addr_size);
		
		sock_tmp = malloc(sizeof(int));
    	*sock_tmp = sock_recv;
		
		// Create new thread for each client
		threadCount++;
		printf("Client %d connected. Creating new thread.\n", threadCount);
		status = pthread_create(&a_thread, NULL, client_handler,
            (void *) sock_tmp);
		
        
    }

    close(sock_recv);
    close(sock_listen);
}

void *client_handler(void *sock_desc)
{
	// used for SQLite
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, strSql[BUF_SIZE];
	const char* data = "Callback function called";
	
	int msg_size, bytes_sent;
	char buf[BUF_SIZE], returnMsg[BUF_SIZE], strArr[10][BUF_SIZE];
	int sock = *(int*)sock_desc;
	int user = threadCount; // Gets user number
	int index, amount, authentication = 0, code =0;
	int count1, count2, count3;
	// count1 = place in buf; count2 = row of entry; count3 = column of entry
	// strArr = 2D array, each row holds a different value from buf
	
	rc = sqlite3_open("group3db.db", &db);
	
	while ((msg_size = recv(sock, buf, BUF_SIZE, 0)) > 0)
	{// Receives data
        buf[msg_size - 1] = 0;
		
		// Clears values in strArr
		int clearIndex1 = 0, clearIndex2 = 0;
		while(clearIndex1 < 10)
		{
			clearIndex2 = 0;
			while(clearIndex2 < BUF_SIZE)
			{
				strArr[clearIndex1][clearIndex2] = 0;
				clearIndex2++;
			}
			clearIndex1++;
		}
		
		count1 = 0; count2 = 0; count3 = 0;
		do
		{// Parse entries in buf by ' ' and store in strArr
			if (buf[count1] == ' ')
			{
				strArr[count2][count3] = '\0';
				count2++;
				count3 = 0;
				count1++;
			}
			else if (buf[count1] == '\0')
			{
				strArr[count2][count3] = '\0';
				break;
			}
			else
			{
				strArr[count2][count3] = buf[count1];
				count3++;
				count1++;
			}
		}
		while (buf[count1] != '\0');
		
		code = convertStrToInt(strArr[0], strlen(strArr[0])); // Turns code into number
		
		// Handles code
		switch (code)
		{
//***********************************************************************************************************
			case 101: 	printf("Client %d: Code 101 Create Account\n", user);
						if (count2 != 6)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Account creation failed: Missing fields\n", user);
							break;
						}
						
						if (strlen(strArr[1]) > 20 || strlen(strArr[2]) > 20 || strlen(strArr[3]) != 4 || 
						    strlen(strArr[4]) != 8 || strlen(strArr[5]) > 9 || strlen(strArr[6]) > 40)
						{// Checks for correct size of entries
							strcpy(returnMsg, "103");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Account creation failed: Entry error\n", user);
							break;
						}
						
						// Check database for record
						strcpy(strSql, "SELECT * FROM customer where ssn='");
						strcat(strSql, strArr[5]);
						strcat(strSql, "'");
						sql = &strSql[0];
						rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
						if (account.ssn[0] != '\0')
						{// Account was found
							strcpy(returnMsg, "105");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Account creation failed: Duplicate Account\n", user);
							
							// Erasing values
							strcpy(account.fName, "\0");
							strcpy(account.lName, "\0");
							strcpy(account.ssn, "\0");
							strcpy(account.pin, "\0");
							strcpy(account.dl, "\0");
							strcpy(account.email, "\0");
							account.balance = 0;
							
							break;
						}
						
						// Save data in DB
						sprintf(strSql, "INSERT INTO customer VALUES('%s','%s','%s','%s','%s','%s',0);",
						        strArr[5], strArr[1], strArr[2], strArr[3], strArr[4], strArr[6]);
						
						sql = &strSql[0];
						
						rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
						strcpy(returnMsg, "104");
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Account creation successful\n", user);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 201: 	printf("Client %d: Code 201 User Authentication\n", user);
						if (count2 != 2)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Authentication failed: Missing fields\n", user);
							break;
						}
						
						// Checks DB for matching account
						// Check database for record
						sprintf(strSql, "SELECT * FROM customer where fName='%s' AND pin='%s'",
						        strArr[1],strArr[2]);
						
						sql = &strSql[0];
						rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
						if (account.ssn[0] != '\0')
						{
							strcpy(returnMsg, "205");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Authentication successful\n", user);
							break;
						}
						else
						{// Authentication failed
							authentication++;
							if (authentication < 10)
							{// Checks for max authentication tries
								strcpy(returnMsg, "203");
								bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
								printf("Client %d: Authentication failed\n", user);
							}
							else
							{
								strcpy(returnMsg, "204");
								bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
								printf("Client %d: Authentication exceeded\n", user);
								strcpy(buf, "801");
							}
						}
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 301: 	printf("Client %d: Code 301 Deposit\n", user);
						if (count2 != 1)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Deposit failed: Missing fields\n", user);
							break;
						}
						
						// Converts strArr amount to int and adds amount to balance of current customer
						amount = convertStrToInt(strArr[1], strlen(strArr[1]));
						account.balance += amount;
						
						sprintf(returnMsg, "303 %d", account.balance);
						
						// Records transaction in DB
						sprintf(strSql, "UPDATE customer SET balance=%d WHERE ssn=%s;", 
						        account.balance, account.ssn);
						
						sql = &strSql[0];
						
						rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
						sprintf(strSql, "INSERT INTO transactions (customer_ssn,amount,type) VALUES ('%s',%d,'deposit');", account.ssn, amount);
						
						sql = &strSql[0];
						
						rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
						// Sends responce to client
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Deposit successful: Amount [%d] New Balance[%d]\n", 
						user, amount, account.balance);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 302: 	printf("Client %d: Code 302 ATM Machine Full\n", user);
						
						sprintf(returnMsg, "405 %d", account.balance);
						
						// Sends responce to client
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Attendant notified\n", user);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 401: 	printf("Client %d: Code 401 Withdraw\n", user);
						if (count2 != 1)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Deposit failed: Missing fields\n", user);
							break;
						}
						
						// Converts strArr amount to int and subtracts amount from balance if possible
						amount = convertStrToInt(strArr[1], strlen(strArr[1]));
						
						if (amount > account.balance)
						{
							sprintf(returnMsg, "404 %d", account.balance);
							printf("Client %d: Withdraw failed. Funds low: Balance[%d]\n", 
						           user, account.balance);
								   
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							break;
						}
						else
						{
							account.balance -= amount;
							sprintf(returnMsg, "403 %d", account.balance);
							printf("Client %d: Withdraw successful: Amount [%d] New Balance[%d]\n", 
							user, amount, account.balance);
							
							// Records transaction in DB
							sprintf(strSql, "UPDATE customer SET balance=%d WHERE ssn=%s;", 
									account.balance, account.ssn);
						
							sql = &strSql[0];
						
							rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
							sprintf(strSql, "INSERT INTO transactions (customer_ssn,amount,type) VALUES ('%s',%d,'withdraw');", account.ssn, amount);
						
							sql = &strSql[0];
						
							rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
							
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						}
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 402: 	printf("Client %d: Code 402 ATM Empty\n", user);
						strcpy(returnMsg, "405");
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Attendant notified\n", user);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 501: 	printf("Client %d: Code 501 Balance Query\n", user);
						
						sprintf(returnMsg, "503 %d", account.balance);
						
						// Sends responce to client
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Balance: %d\n", user, account.balance);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 601: 	printf("Client %d: Code 601 Transactions Query\n", user);
						if (count2 != 1)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: transaction request failed: Missing fields\n", user);
							break;
						}
						
						char tempStr[BUF_SIZE];
						int trans;
						
						sprintf(strSql,"SELECT * from transactions where customer_ssn='%s' ORDER BY id DESC LIMIT 5",
						                account.ssn);
						
						sql = &strSql[0];
						
						account.numTransactions = 0;
						
						rc = sqlite3_exec(db, sql, getTransactions, (void*)data, &zErrMsg); // Executes SQL statement
						
						trans = convertStrToInt(strArr[1], strlen(strArr[1]));
						
						if (trans > account.numTransactions)
							trans = account.numTransactions;
						
						sprintf(returnMsg, "603 %d", trans);
						
						index = 0;
						
						while (index < trans)
						{
							sprintf(tempStr, " %d %s %s", index + 1, 
							        account.userTransactions[index].description,
									account.userTransactions[index].amount);
									
							strcat(returnMsg, tempStr);
							index++;
						}
						
						// Sends responce to client
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Transactions sent\n", user);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 701: 	printf("Client %d: Code 701 Buy Stamps\n", user);
						if (count2 != 1)
						{// Checks for correct number of entries
							strcpy(returnMsg, "908");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Deposit failed: Missing fields\n", user);
							break;
						}
						
						// Converts strArr amount to int and subtracts amount from balance if possible
						amount = convertStrToInt(strArr[1], strlen(strArr[1]));
						
						if (amount > account.balance)
						{
							sprintf(returnMsg, "703 %d", account.balance);
							printf("Client %d: Stamps Withdraw failed. Funds low: Balance[%d]\n", 
						           user, account.balance);
								   
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							break;
						}
						else
						{
							account.balance -= amount;
							sprintf(returnMsg, "704 %d", account.balance);
							printf("Client %d: Stamps Withdraw successful: Amount [%d] New Balance[%d]\n", 
							user, amount, account.balance);
							
							// Records transaction in DB
							sprintf(strSql, "UPDATE customer SET balance=%d WHERE ssn=%s;", 
									account.balance, account.ssn);
						
							sql = &strSql[0];
						
							rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
						
							sprintf(strSql, "INSERT INTO transactions (customer_ssn,amount,type) VALUES ('%s',%d,'stamps');", account.ssn, amount);
						
							sql = &strSql[0];
						
							rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg); // Executes SQL statement
							
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						}
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 702: 	printf("Client %d: Code 702 Out of stamps\n", user);
						strcpy(returnMsg, "705");
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Attendant notified\n", user);
						
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			case 801: 	printf("Client %d: Code 801 Logout\n", user);
			
						break;
//***********************************************************************************************************
//***********************************************************************************************************
			default :	strcpy(returnMsg, "909");
						bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
						printf("Client %d: Unknown code %d\n", user, code);
						
						break;
//***********************************************************************************************************
		}// end switch
		
        if (strcmp(buf, "801") == 0)
		{
			strcpy(returnMsg, "803");
			bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
            break;
		}
		
    }// end while
	
	printf("Client %d disconnected. Exiting thread.\n", user);
	sqlite3_close(db);
    close(sock);
	free(sock_desc);
	threadCount--;
    pthread_exit("Thank you for the CPU time");
}

static int getRecord(void *NotUsed, int argc, char **argv, char **azColName){
	
	strcpy(account.ssn, argv[0]);
	strcpy(account.fName, argv[1]);
	strcpy(account.lName, argv[2]);
	strcpy(account.pin, argv[3]);
	strcpy(account.dl, argv[4]);
	strcpy(account.email, argv[5]);
	account.balance = convertStrToInt(argv[6], strlen(argv[6]));
		
	return 0;
}

static int getTransactions(void *NotUsed, int argc, char **argv, char **azColName){
	
	strcpy(account.userTransactions[account.numTransactions].amount, argv[2]);
	strcpy(account.userTransactions[account.numTransactions].description, argv[3]);
	
	account.numTransactions++;
	
	return 0;
}

int convertStrToInt(char msg[BUF_SIZE], int size)
{
	int numTotal = 0, numTemp = 0, i = 0;
	
	while (i < size)
	{
		switch (msg[i])
		{
			case '0' : numTemp = 0;
					   break;
			case '1' : numTemp = 1;
					   break;
			case '2' : numTemp = 2;
					   break;
			case '3' : numTemp = 3;
					   break;
			case '4' : numTemp = 4;
					   break;
			case '5' : numTemp = 5;
					   break;
			case '6' : numTemp = 6;
					   break;
			case '7' : numTemp = 7;
					   break;
			case '8' : numTemp = 8;
					   break;
			case '9' : numTemp = 9;
					   break;
		}
		
		numTotal += numTemp * power(10, size - i - 1);
		
		i++;
	}
	
	return numTotal;
}

int findSpace(char msg[BUF_SIZE])
{
	int count = 0;
	
	while (msg[count] != ' ' && msg[count] != '\0')
	{
		count++;
	}
	
	if (msg[count] == '\0')
		return 0;
	else
		return count;
}

int power(int num, int pow)
{	
	int tempNum = num;
	if (pow == 0)
		return 1;
	
	while(pow > 1)
	{
		num *= tempNum;
		pow--;
	}
	return num;
}

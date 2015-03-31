// compile with: gcc -pthread -o s server.c 
// Multiple server-client connection; two way communication
	
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>	// system type defintions
#include <sys/socket.h>	// network system functions
#include <netinet/in.h>	// protocol & struct definitions 
#include <pthread.h>    // threading

#define BUF_SIZE	1024
#define LISTEN_PORT	60000
// #define LISTEN_POR 53645

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
	bool active;
	int balance;
	int numTransactions;
	struct transactions userTransactions[5];
};

int threadCount = 0, numAccounts = 0;
struct userAccount userAccounts[100];


void *client_handler(void *arg);
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
    struct timeval		timeout={0,0};
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
	int msg_size, bytes_sent, code = 0;
	char buf[BUF_SIZE], returnMsg[BUF_SIZE], strArr[10][BUF_SIZE], ssn[10];
	int sock = *(int*)sock_desc;
	int user = threadCount; // Gets user number
	int tempInt = 0, tempCount = 0, trans = 0;
	int userID = 0; // User location in useAccounts array
	int count1, count2, count3, index, authentication = 0, amount;
	int i,flagCheck,toSwitch;
	// count1 = place in buf; count2 = row of entry; count3 = column of entry
	// strArr = 2D array, each row holds a different value from buf
	
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
		
		if(strncmp(strArr[0], "quit", 4) == 0)
			code=801;
			
		else
			code = convertStrToInt(strArr[0], strlen(strArr[0])); // Turns code into number
		
//Flag checking and no active account error checking
		toSwitch=0;
		if(code == 801)
			toSwitch=1;
		//If no accounts have been created let client know unless they're creating one
		else
		if(numAccounts==0){
			if(code != 101 ){
				strcpy(returnMsg, "No accounts on server. Please create one before continuing");
				bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
				printf("Client %d: No accounts on server.\n", user);
			}
			else
				toSwitch=1;
		}
		//Check to see if an account has been authenticated
		else
		if(code != 201 || code !=101){
			flagCheck=0;
			for(i=0;i<numAccounts;i++){
				if(userAccounts[i].active==true)
					flagCheck++;
			}
			if(flagCheck == 0 ){
				strcpy(returnMsg, "No account authenticated. Please authenticate before continuing");
				bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
				printf("Client %d: No account authenticated\n", user);
			}
			else
				toSwitch=1;
		}
		else
			toSwitch=1;
//end flag checking and handle codes		
		if(toSwitch==1)
			switch (code)
			// Handles code
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
							
							index = 0;
							while (index < numAccounts)
							{// Checks for duplicate account
								if (!strcmp(userAccounts[index].ssn, strArr[5]))
								{
									strcpy(returnMsg, "105");
									bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
									printf("Client %d: Account creation failed: Duplicate Account\n", user);
									break;
								}
								index++;
							}
							
							if (index == numAccounts)
							{// Saves new account
								strcpy(userAccounts[index].fName, strArr[1]);
								strcpy(userAccounts[index].lName, strArr[2]);
								strcpy(userAccounts[index].pin, strArr[3]);
								strcpy(userAccounts[index].dl, strArr[4]);
								strcpy(userAccounts[index].ssn, strArr[5]);
								strcpy(userAccounts[index].email, strArr[6]);
								userAccounts[index].balance = 0;
								userAccounts[index].numTransactions = 0;
								
								//Flag to label account as active to deposit/withdraw
								userAccounts[index].active=true;	
								
								strcpy(returnMsg, "104");
									bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
								printf("Client %d: Account creation successful\n", user);
								strcpy(ssn, strArr[5]); // Stores ssn for future validation
								userID = index; // Index of user in userAccounts
								numAccounts++;
							}
							
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
							
							index = 0;
							while (index < numAccounts)
							{// Checks for matching fName and pin
								if (!strcmp(userAccounts[index].fName, strArr[1]) && 
									  !strcmp(userAccounts[index].pin, strArr[2]))
								{
									strcpy(returnMsg, "205");
									bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
									printf("Client %d: Authentication successful\n", user);
									strcpy(ssn, userAccounts[index].ssn); // Stores ssn for future validation
									userID = index; // Index of user in userAccounts
									break;
								}
								index++;
							}
							
							if (index == numAccounts)
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
							userAccounts[userID].balance += amount;
							
							tempInt = userAccounts[userID].balance, tempCount = 0;
							while (tempInt > 0)
							{// Gets the size of balance
								tempInt /= 10;
								tempCount++;
							}
							
							strcpy(returnMsg, "303  ");
							returnMsg[4 + tempCount] = 0;
							tempInt = userAccounts[userID].balance;

							while(tempCount > 0)
							{// Turns number back into char array
								switch(tempInt % 10)
								{
									case 0 : returnMsg[3 + tempCount] = '0';
											 break;
									case 1 : returnMsg[3 + tempCount] = '1';
											 break;
									case 2 : returnMsg[3 + tempCount] = '2';
											 break;
									case 3 : returnMsg[3 + tempCount] = '3';
											 break;
									case 4 : returnMsg[3 + tempCount] = '4';
											 break;
									case 5 : returnMsg[3 + tempCount] = '5';
											 break;
									case 6 : returnMsg[3 + tempCount] = '6';
											 break;
									case 7 : returnMsg[3 + tempCount] = '7';
											 break;
									case 8 : returnMsg[3 + tempCount] = '8';
											 break;
									case 9 : returnMsg[3 + tempCount] = '9';
											 break;
								}
								tempCount--;
								tempInt /= 10;
							}
							
							// Records transaction and move other transactions up a place in array
							strcpy(userAccounts[userID].userTransactions[4].description, 
								   userAccounts[userID].userTransactions[3].description);
							strcpy(userAccounts[userID].userTransactions[4].amount, 
								   userAccounts[userID].userTransactions[3].amount);
							strcpy(userAccounts[userID].userTransactions[3].description, 
								   userAccounts[userID].userTransactions[2].description);
							strcpy(userAccounts[userID].userTransactions[3].amount, 
								   userAccounts[userID].userTransactions[2].amount);
							strcpy(userAccounts[userID].userTransactions[2].description, 
								   userAccounts[userID].userTransactions[1].description);
							strcpy(userAccounts[userID].userTransactions[2].amount, 
								   userAccounts[userID].userTransactions[1].amount);
							strcpy(userAccounts[userID].userTransactions[1].description, 
								   userAccounts[userID].userTransactions[0].description);
							strcpy(userAccounts[userID].userTransactions[1].amount, 
								   userAccounts[userID].userTransactions[0].amount);
							strcpy(userAccounts[userID].userTransactions[0].description, 
								   "Deposit");
							strcpy(userAccounts[userID].userTransactions[0].amount, 
								   strArr[1]);
							if (userAccounts[userID].numTransactions < 5)
								userAccounts[userID].numTransactions++;
							
							
							// Sends responce to client
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Deposit successful: Amount [%d] New Balance[%d]\n", 
							user, amount, userAccounts[userID].balance);
							
							break;
	//***********************************************************************************************************
	//***********************************************************************************************************
				case 302: 	printf("Client %d: Code 302 ATM Machine Full\n", user);
							tempInt = userAccounts[userID].balance, tempCount = 0;
							while (tempInt > 0)
							{// Gets the size of balance
								tempInt /= 10;
								tempCount++;
							}
							
							strcpy(returnMsg, "405 ");
							returnMsg[4 + tempCount] = 0;
							tempInt = userAccounts[userID].balance;

							while(tempCount > 0)
							{// Turns number back into char array
								switch(tempInt % 10)
								{
									case 0 : returnMsg[3 + tempCount] = '0';
											 break;
									case 1 : returnMsg[3 + tempCount] = '1';
											 break;
									case 2 : returnMsg[3 + tempCount] = '2';
											 break;
									case 3 : returnMsg[3 + tempCount] = '3';
											 break;
									case 4 : returnMsg[3 + tempCount] = '4';
											 break;
									case 5 : returnMsg[3 + tempCount] = '5';
											 break;
									case 6 : returnMsg[3 + tempCount] = '6';
											 break;
									case 7 : returnMsg[3 + tempCount] = '7';
											 break;
									case 8 : returnMsg[3 + tempCount] = '8';
											 break;
									case 9 : returnMsg[3 + tempCount] = '9';
											 break;
								}
								tempCount--;
								tempInt /= 10;
							}
							
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
							
							if (amount > userAccounts[userID].balance)
							{
								strcpy(returnMsg, "404  ");
								printf("Client %d: Withdraw failed. Funds low: Balance[%d]\n", 
							user, userAccounts[userID].balance);
							}
							else
							{
								userAccounts[userID].balance -= amount;
								strcpy(returnMsg, "403  ");
								printf("Client %d: Withdraw successful: Amount [%d] New Balance[%d]\n", 
								user, amount, userAccounts[userID].balance);
							}
							
							tempInt = userAccounts[userID].balance, tempCount = 0;
							while (tempInt > 0)
							{// Gets the size of balance
								tempInt /= 10;
								tempCount++;
							}
							
							returnMsg[4 + tempCount] = 0;
							tempInt = userAccounts[userID].balance;

							while(tempCount > 0)
							{// Turns number back into char array
								switch(tempInt % 10)
								{
									case 0 : returnMsg[3 + tempCount] = '0';
											 break;
									case 1 : returnMsg[3 + tempCount] = '1';
											 break;
									case 2 : returnMsg[3 + tempCount] = '2';
											 break;
									case 3 : returnMsg[3 + tempCount] = '3';
											 break;
									case 4 : returnMsg[3 + tempCount] = '4';
											 break;
									case 5 : returnMsg[3 + tempCount] = '5';
											 break;
									case 6 : returnMsg[3 + tempCount] = '6';
											 break;
									case 7 : returnMsg[3 + tempCount] = '7';
											 break;
									case 8 : returnMsg[3 + tempCount] = '8';
											 break;
									case 9 : returnMsg[3 + tempCount] = '9';
											 break;
								}
								tempCount--;
								tempInt /= 10;
							}
							
							// Records transaction and move other transactions up a place in array
							strcpy(userAccounts[userID].userTransactions[4].description, 
								   userAccounts[userID].userTransactions[3].description);
							strcpy(userAccounts[userID].userTransactions[4].amount, 
								   userAccounts[userID].userTransactions[3].amount);
							strcpy(userAccounts[userID].userTransactions[3].description, 
								   userAccounts[userID].userTransactions[2].description);
							strcpy(userAccounts[userID].userTransactions[3].amount, 
								   userAccounts[userID].userTransactions[2].amount);
							strcpy(userAccounts[userID].userTransactions[2].description, 
								   userAccounts[userID].userTransactions[1].description);
							strcpy(userAccounts[userID].userTransactions[2].amount, 
								   userAccounts[userID].userTransactions[1].amount);
							strcpy(userAccounts[userID].userTransactions[1].description, 
								   userAccounts[userID].userTransactions[0].description);
							strcpy(userAccounts[userID].userTransactions[1].amount, 
								   userAccounts[userID].userTransactions[0].amount);
							strcpy(userAccounts[userID].userTransactions[0].description, 
								   "Withdraw");
							strcpy(userAccounts[userID].userTransactions[0].amount, 
								   strArr[1]);
							if (userAccounts[userID].numTransactions < 5)
								userAccounts[userID].numTransactions++;
							
							// Sends responce to client
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							
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
							tempInt = userAccounts[userID].balance, tempCount = 0;
							while (tempInt > 0)
							{// Gets the size of balance
								tempInt /= 10;
								tempCount++;
							}
							
							strcpy(returnMsg, "503 ");
							returnMsg[4 + tempCount] = 0;
							tempInt = userAccounts[userID].balance;

							while(tempCount > 0)
							{// Turns number back into char array
								switch(tempInt % 10)
								{
									case 0 : returnMsg[3 + tempCount] = '0';
											 break;
									case 1 : returnMsg[3 + tempCount] = '1';
											 break;
									case 2 : returnMsg[3 + tempCount] = '2';
											 break;
									case 3 : returnMsg[3 + tempCount] = '3';
											 break;
									case 4 : returnMsg[3 + tempCount] = '4';
											 break;
									case 5 : returnMsg[3 + tempCount] = '5';
											 break;
									case 6 : returnMsg[3 + tempCount] = '6';
											 break;
									case 7 : returnMsg[3 + tempCount] = '7';
											 break;
									case 8 : returnMsg[3 + tempCount] = '8';
											 break;
									case 9 : returnMsg[3 + tempCount] = '9';
											 break;
								}
								tempCount--;
								tempInt /= 10;
							}
							
							// Sends responce to client
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Balance: %d\n", user, userAccounts[userID].balance);
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
							
							trans = convertStrToInt(strArr[1], strlen(strArr[1]));
							strcpy(returnMsg, "603");
							index = 0;
							
							if (trans > userAccounts[userID].numTransactions)
								trans = userAccounts[userID].numTransactions;
							
							switch(trans)
							{
								case 1 : strcat(returnMsg, " 1");
										 break;
								case 2 : strcat(returnMsg, " 2");
										 break;
								case 3 : strcat(returnMsg, " 3");
										 break;
								case 4 : strcat(returnMsg, " 4");
										 break;
								case 5 : strcat(returnMsg, " 5");
										 break;
							}
							
							while (index < trans)
							{
								switch(index)
								{
									case 0 : strcat(returnMsg, " 1 ");
											break;
									case 1 : strcat(returnMsg, " 2 ");
											break;
									case 2 : strcat(returnMsg, " 3 ");
											break;
									case 3 : strcat(returnMsg, " 4 ");
											break;
									case 4 : strcat(returnMsg, " 5 ");
											break;
								}
								
								strcat(returnMsg, userAccounts[userID].userTransactions[index].description);
								strcat(returnMsg, " ");
								strcat(returnMsg, userAccounts[userID].userTransactions[index].amount);
								index++;
							}
							
							strcat(returnMsg, "\0");
							
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
							
							if (amount > userAccounts[userID].balance)
							{
								strcpy(returnMsg, "703");
								printf("Client %d: Stamps Withdraw failed. Funds low: Balance[%d]\n", 
								user, userAccounts[userID].balance);
								bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
								break;
							}
							else
							{
								userAccounts[userID].balance -= amount;
								strcpy(returnMsg, "704  ");
								printf("Client %d: Stamps Withdraw successful: Amount [%d] New Balance[%d]\n", 
								user, amount, userAccounts[userID].balance);
							}
							
							tempInt = userAccounts[userID].balance, tempCount = 0;
							while (tempInt > 0)
							{// Gets the size of balance
								tempInt /= 10;
								tempCount++;
							}
							
							returnMsg[4 + tempCount] = 0;
							tempInt = userAccounts[userID].balance;

							while(tempCount > 0)
							{// Turns number back into char array
								switch(tempInt % 10)
								{
									case 0 : returnMsg[3 + tempCount] = '0';
											 break;
									case 1 : returnMsg[3 + tempCount] = '1';
											 break;
									case 2 : returnMsg[3 + tempCount] = '2';
											 break;
									case 3 : returnMsg[3 + tempCount] = '3';
											 break;
									case 4 : returnMsg[3 + tempCount] = '4';
											 break;
									case 5 : returnMsg[3 + tempCount] = '5';
											 break;
									case 6 : returnMsg[3 + tempCount] = '6';
											 break;
									case 7 : returnMsg[3 + tempCount] = '7';
											 break;
									case 8 : returnMsg[3 + tempCount] = '8';
											 break;
									case 9 : returnMsg[3 + tempCount] = '9';
											 break;
								}
								tempCount--;
								tempInt /= 10;
							}
							
							// Records transaction and move other transactions up a place in array
							strcpy(userAccounts[userID].userTransactions[4].description, 
								   userAccounts[userID].userTransactions[3].description);
							strcpy(userAccounts[userID].userTransactions[4].amount, 
								   userAccounts[userID].userTransactions[3].amount);
							strcpy(userAccounts[userID].userTransactions[3].description, 
								   userAccounts[userID].userTransactions[2].description);
							strcpy(userAccounts[userID].userTransactions[3].amount, 
								   userAccounts[userID].userTransactions[2].amount);
							strcpy(userAccounts[userID].userTransactions[2].description, 
								   userAccounts[userID].userTransactions[1].description);
							strcpy(userAccounts[userID].userTransactions[2].amount, 
								   userAccounts[userID].userTransactions[1].amount);
							strcpy(userAccounts[userID].userTransactions[1].description, 
								   userAccounts[userID].userTransactions[0].description);
							strcpy(userAccounts[userID].userTransactions[1].amount, 
								   userAccounts[userID].userTransactions[0].amount);
							strcpy(userAccounts[userID].userTransactions[0].description, 
								   "Stamps");
							strcpy(userAccounts[userID].userTransactions[0].amount, 
								   strArr[1]);
							if (userAccounts[userID].numTransactions < 5)
								userAccounts[userID].numTransactions++;
							
							// Sends responce to client
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							
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
							strcpy(returnMsg, "803");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							
							break;
	//***********************************************************************************************************
	//***********************************************************************************************************
				default :	strcpy(returnMsg, "909");
							bytes_sent = send(sock, returnMsg, strlen(returnMsg), 0);
							printf("Client %d: Unknown code %d\n", user, code);
							
							break;
	//***********************************************************************************************************
			}// end switch
			
    }// end while
	
	printf("Client %d disconnected. Exiting thread.\n", user);
    pthread_exit("Thank you for the CPU time");
	close(sock);
	free(sock_desc);
	threadCount--;
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

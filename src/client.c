// Multiple server-client connection

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#define BUF_SIZE	1024
// #define	SERVER_IP	"129.120.151.98" // cse05
// #define	SERVER_IP	"129.120.151.97" // cse04
#define	SERVER_IP	"129.120.151.96" // cse03
// #define	SERVER_IP	"129.120.151.95" // cse02
// #define	SERVER_IP	"129.120.151.94" // cse01

#define SERVER_PORT	60000
// #define SERVER_PORT 53645

void userDirections();
void returnMsg(int code, int num);
int convertStrToInt(char msg[BUF_SIZE], int size);
int power(int num, int pow);

void userDirections(){
	printf("********************************************************************* \n");
	printf("********************************************************************* \n");
	
	printf("\nUsage of this ATM is as follows: \n");
	printf("\nCreate account, code 101 \n");
	printf("Format: <101 firstName lastName 4-digit-Pin DL SSN emailAddress> \n");

	printf("\nAuthentication, code 201 \n");
	printf("Format: <201 firstName Pin> \n");
	
	printf("\nDeposit, code 301 \n");
	printf("Format: <301 Amount> \n");
	
	printf("\nWithdraw, code 401 \n");
	printf("Format: <401 Amount> \n");
	
	printf("\nBalance, code 501 \n");
	printf("Format: <501> \n");
	
	printf("\nShow Transactions, code 601 \n");
	printf("Format: <601 numOfTransactions(1-5)> \n");
	
	printf("\nBuy Stamps, code 701 \n");
	printf("Format: <701 Amount> \n");
	
	printf("\nLogout, code 801 \n");
	printf("Format: <801> \n\n");
	
	printf("********************************************************************* \n");
	printf("********************************************************************* \n");
}

void returnMsg(int code, int balance){
	switch (code){
		case 103:
			printf("Account creation failed: Entry error\n");
			break;
		case 104:
			printf("Account creation successful\n");
			break;
		case 105:
			printf("Account creation failed: Duplicate Account\n");
			break;
		case 203:
			printf("Authentication failed\n");
			break;
		case 204:
			printf("Authentication exceeded\n");
			break;
		case 205:
			printf("Authentication successful\n");
			break;
		case 302:
			printf("ATM Machine Full\nAttendant notified\n");
			break;
		case 303:
			printf("Deposit successful\n");
			printf("Current balance: %d\n",balance);
			break;
		case 304:
			//failure + balance
			break;
		case 402:
			printf(" ATM Empty\nAttendant notified\n");
			break;
		case 403:
			printf("Withdraw successful\n");
			printf("Current balance: %d\n",balance);
			break;
		case 404:
			printf("Withdraw failed. Funds low\n");
			printf("Current balance: %d\n",balance);
			break;
		case 405:
			printf(" ATM Empty\nAttendant notified\n");
			break;
		case 503:
			printf("Balance: %d\n",balance);
			break;
		case 603:
			//printf("Transactions sent\n");
			break;
		case 702:
			printf("Out of stamps\nAttendant notified\n");
			break;
		case 703:
			printf("Stamps Withdraw failed. Funds low\n");
			break;
		case 704:
			printf("Stamps Withdraw successful\n"); 
			printf("Balance: %d\n",balance);
			break;
		case 705:
			printf("Out of stamps\nAttendant notified\n");
			break;
		case 803:
			printf("Client disconnected.\nGood Bye\n");
			break;
		case 908:
			printf("Error: Missing fields\n");
			break;
		default:
			printf("Unknown Code\n");
			break;
	}
		
}

int convertStrToInt(char msg[BUF_SIZE], int size)
{
	int numTotal = 0, numTemp = 0, i = 0;
	if(size >3)
		i=3;
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

int main(int argc, char *argv[])
{
    int sock_send;
    struct sockaddr_in	addr_send;
    int	i,returnCode,balance;
    char text[BUF_SIZE], buf[BUF_SIZE];
    int	send_len, bytes_sent, bytes_received;

	// create socket for sending data
    sock_send = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_send < 0)
	{
        printf("socket() failed\n");
        exit(0);
    }

	// create socket address structure to connect to
    memset(&addr_send, 0, sizeof (addr_send)); /* zero out structure */
    addr_send.sin_family = AF_INET; /* address family */
    addr_send.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr_send.sin_port = htons((unsigned short)SERVER_PORT);

	// connect to the server
    i = connect(sock_send, (struct sockaddr *) &addr_send, sizeof (addr_send));
	if (i < 0)
	{
        printf("connect() failed\n");
		close(sock_send);
        exit(0);
    }

	//Prompts user is they would like to display 
	//directions on how to use the ATM.
	printf("Would you like to see the directions? ");
	fgets(text, sizeof(text), stdin);
	if (strncmp(text,"yes",3) == 0 || strncmp(text,"y",1) == 0 )
		if(text[4] == '\0' || text[2] == '\0' )
			userDirections();
	
    while (1)
	{
        // Sample to show how to send data
		// Use "shutdown" to close connection with server and "quit" to exit client
        printf("Send? ");
        //scanf("%s",text);
        fgets(text, sizeof(text), stdin);
	
		
		// Get request from user
		// TODO get request

		// Send data
        strcpy(buf,text);
        send_len = strlen(text);
        if(bytes_sent = send(sock_send,buf, send_len, 0))
		printf("Sent: %s\n", buf);
		
		// Receive data
		bytes_received = recv(sock_send, buf, BUF_SIZE, 0);
        buf[bytes_received] = 0;
        //printf("Received: %s\n", buf);
		
		// Turns code into number
		returnCode = convertStrToInt(buf, 3);
		if(returnCode == 603 )
			printf("Received: %s\n", buf);
		else
			balance = convertStrToInt(buf, strlen(buf));
		
		//printf("return code = %d\n",returnCode);
		//printf("test = %d\n",test);
		
		//Handles returnCode
		returnMsg(returnCode,balance);
		
		if(strcmp(buf,"803") == 0)
			break;
    }

    close(sock_send);
}

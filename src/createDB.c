// Compile with: gcc createDB.c -l sqlite3 -o db

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char *argv[])
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = "Callback function called";
	
	// Opens DB "group3db"
	rc = sqlite3_open("group3db.db", &db);

	if( rc )
	{	// Checks for error in sql code
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}
	
	// Create customer table, first deletes if already exists
	sql = "DROP TABLE IF EXISTS `customer`;" \
		  "CREATE TABLE IF NOT EXISTS `customer` (" \
		  "`SSN` VARCHAR(9) PRIMARY KEY NOT NULL," \
		  "`firstName` VARCHAR(45) NOT NULL," \
		  "`lastName` VARCHAR(45) NOT NULL," \
		  "`pin` VARCHAR(4) NOT NULL," \
		  "`DL` VARCHAR(8) NOT NULL," \
		  "`emailAddress` VARCHAR(40) NOT NULL," \
		  "`balance` DECIMAL(8,2) NOT NULL);";
		  
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); // Executes SQL statement
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(0);
	}
	else
	{
		fprintf(stdout, "Table created successfully\n");
	}
	
	// Create transactions table, first deletes if already exists
	sql = "DROP TABLE IF EXISTS `transactions`;" \
		  "CREATE TABLE IF NOT EXISTS `transactions` (" \
		  "`id` INTEGER PRIMARY KEY AUTOINCREMENT," \
		  "`customer_SSN` VARCHAR(9) NOT NULL," \
		  "`amount` DECIMAL(8,2) NOT NULL," \
		  "`type` VARCHAR(45) NOT NULL)";
		
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); // Executes SQL statement
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(0);
	}
	else
	{
		fprintf(stdout, "Table created successfully\n");
	}
	
	// Inserts values into both tables
	sql = "INSERT INTO customer (SSN,firstName,lastName,pin,DL,emailAddress,balance) " \
		  "VALUES ('112345656','Jason','Byrd','1234','12345678','xjb@gmail.com',500000.00);" \
		  "INSERT INTO customer (SSN,firstName,lastName,pin,DL,emailAddress,balance) " \
		  "VALUES ('223456767','Juan','Hernandez','2345','23456789','xjh@gmail.com',5000.00);" \
		  "INSERT INTO customer (SSN,firstName,lastName,pin,DL,emailAddress,balance) " \
		  "VALUES ('334567878','Ben','Garside','3456','34567891','xbg@gmail.com',500.00);" \
		  "INSERT INTO customer (SSN,firstName,lastName,pin,DL,emailAddress,balance) " \
		  "VALUES ('111111111','Jack','Bauer','1111','11111111','jack@gmail.com',5000000.00);" \
		  "INSERT INTO transactions (customer_SSN,amount,type) " \
		  "VALUES ('111111111', 5000000.00, 'deposit');" \
		  "INSERT INTO transactions (customer_SSN,amount,type) " \
		  "VALUES ('112345656', 500000.00, 'deposit');" \
		  "INSERT INTO transactions (customer_SSN,amount,type) " \
		  "VALUES ('223456767', 5000.00, 'deposit');" \
		  "INSERT INTO transactions (customer_SSN,amount,type) " \
		  "VALUES ('334567878', 500.00, 'deposit');";
		
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); // Executes SQL statement
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Records created successfully\n");
	}
	
	// Checks customer table by printing records
	sql = "SELECT * from customer";

	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); // Executes SQL statement
		
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Operation done successfully\n");
	}
	
	// Checks transactions table by printing records
	sql = "SELECT * from transactions";

	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); // Executes SQL statement
		
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Operation done successfully\n");
	}
		
	
	// closes database
	sqlite3_close(db);
	return 0;
}

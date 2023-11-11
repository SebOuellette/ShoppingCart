#ifndef DB_HPP
#define DB_HPP

#include <sqlite3.h>
#include "defines.hpp"
#include "product.hpp"
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
// Using sqlite https://www.geeksforgeeks.org/sql-using-c-c-and-sqlite/

// debug
#define LOG_SUCCESSFUL_QUERIES

class DB {
private:
	sqlite3* _db;

public:
	DB() {
		this->init();
	}

	~DB() {
		sqlite3_close(_db);
	}
	
	// General purpose query. You can use a labmda function as the second argument
	// The lambda function parameters are (void* data, int argc, char** argv, char** columnNames)
	//			The lambda function is what this \/ \/ \/ \/ \/ mess is about
	bool run(string queryString, int(*callback)(void*, int, char**, char**) = NULL, void* data = NULL) {
		char* errorMsg;
		
		// Perform the query
		int exRet = sqlite3_exec(this->_db, queryString.c_str(), callback, data, &errorMsg);

		// Check if an error was reported
		if (exRet != SQLITE_OK) {
			cerr << "Error performing query: " << errorMsg << std::endl;
			sqlite3_free(errorMsg);
		}
#ifdef LOG_SUCCESSFUL_QUERIES
		else {
			cerr << "Query Successful." << endl;
		}
#endif

		// Returns the exit status. AKA the status of the last query
		return (exRet == SQLITE_OK);
	}

	// Initialize the database table
	bool init() {
		string createTables[TABLES] = {
			"CREATE TABLE IF NOT EXISTS Users (id int NOT NULL UNIQUE, cartid int NOT NULL UNIQUE, name varchar(128) NOT NULL unique, passhash varchar(1024) NOT NULL);",
			"CREATE TABLE IF NOT EXISTS Carts (id int NOT NULL UNIQUE, userid int NOT NULL UNIQUE);",
			"CREATE TABLE IF NOT EXISTS Products (id int NOT NULL unique, cartid int NOT NULL, sellerid int NOT NULL, name varchar(128) NOT NULL, description varchar(4096) NOT NULL, quantity int NOT NULL, unitcost double NOT NULL, imgurl varchar(512) NOT NULL);"
		};

		// Open Database
		int exit = sqlite3_open(DB_PATH, &(this->_db));

		// Ensure the database opens correctly
		if (exit != SQLITE_OK) {
			cerr << "Database failed to open." << endl;
			
			throw 1;
		}

		// Now attempt to run the defined queries
		for (int i=0;i<TABLES;i++) {
			// Just run each query without a callback, there's no need here
			this->run(createTables[i], NULL);
		}

		// Returns the final exit status. AKA the status of the last query
		return (exit == SQLITE_OK);
	}

	// Get list of products by user id
	vector<Product> loadCartProducts(ID userID) {
		stringstream query;
		query << "SELECT Products.*, Users.id as userid FROM Products INNER JOIN Carts ON Carts.id=Products.cartid INNER JOIN Users ON Users.id=Carts.userid WHERE Users.id=" << userID <<  ";";
		//cout << "Running: " << query.str() << endl;

		vector<Product> products;

		this->run(query.str(), [](void* data, int argc, char** argv, char** colNames){
			// This same block of code will run for each SQL result
			//cout << "Found: " << argc << " rows" << endl;

			vector<Product>* products = (vector<Product>*)data;

			Product p;

			// Loop through each row
			for (int r=0;r<argc;r++) {
	
				// Build a product
				if (strcmp(colNames[r], "id") == 0) {
					p.id = atoi(argv[r]);
				} else if (strcmp(colNames[r], "sellerid") == 0) {
					p.sellerID = atoi(argv[r]);
				} else if (strcmp(colNames[r], "name") == 0) {
					int length = strlen(argv[r]) + 1;
					strncpy(p.name, argv[r], length);
				} else if (strcmp(colNames[r], "description") == 0) {
					int length = strlen(argv[r]) + 1;
					strncpy(p.description, argv[r], length);
				} else if (strcmp(colNames[r], "imgurl") == 0) {
					int length = strlen(argv[r]);
					strncpy(p.imgurl, argv[r], length);
				} else if (strcmp(colNames[r], "quantity") == 0) {
					p.quantity = atoi(argv[r]);
				} else if (strcmp(colNames[r], "unitcost") == 0) {
					p.price = atof(argv[r]);
				} 

				//cout << colNames[r] << " : " << argv[r] << endl;
			}

			products->push_back(p);
			return 0;
		}, (void*)&products);

		// Return the list of products
		return products;
	}
};

#endif

// Useful queries

	// Example 1: Find user using product ID
	// SELECT Users.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid INNER JOIN Users ON Users.id=Carts.userid WHERE Products.id=334;

	// Example 2: Find all products in a cart using the userID
	// SELECT Products.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid where Carts.userid=323

	// Example 3: Find all products in a cart using the username
	// SELECT Products.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid INNER JOIN Users ON Users.id=Carts.userid WHERE Users.name="John Ron"
#define CROW_MAIN

#include <sqlite3.h>
// Using sqlite https://www.geeksforgeeks.org/sql-using-c-c-and-sqlite/

#include "crow_all.h"
#include <regex>
#include <iostream>
#include <string>
using namespace std;
using namespace crow;

// Program Defines
#define TABLES 3
#define DB_PATH "../cart.db" // will create cart.db in the shared folder


// Module IP Addresses
#define PROFILE "http://localhost:8081"

// Product Defines
#define NAME_LENGTH 128
#define DESCRIPTION_LENGTH 4096

// ID variable type is an unsigned 64-bit integer
typedef unsigned long long int ID;

// Structs
typedef struct _Product {
	ID id;
	ID sellerID;
	char name[NAME_LENGTH];
	char description[DESCRIPTION_LENGTH];
	float price;

	unsigned int quantity = 1;

	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const { 
		return id;
	}
} Product;


// Classes
class Cart {
private:
	ID _id; // { get }
	ID _userID; // { get }
	vector<Product> _products; // { get, set }

public:
	Cart(ID id, ID userID)
	{
		this->_id = id;
		this->_userID = userID;
	}

	void operator=(Cart c) {
		this->_id = c.getID();
		this->_userID = c.getUserID();
		this->_products = c._products;
	}

	// Getters/Setters
	ID getID() {
		return this->_id;
	}

	ID getUserID() {
		return this->_userID;
	}

	void addProduct(Product newP) {
		// TODO
		// Check if product already exists in vector
		 /// If so, increase its quantity by newP.quantity

		// If not, push product to vector

		// (If we want to just add by product id, then we would also need to search for everything first)
	}

	// Find the number of unique elements in the _products vector. Having a quantity of > 1 does not count as more than 1 element
	int productCount() {
		return this->_products.size();
	}

	// Return a reference to the product at index index
	Product& at(ID index) {
		int size = this->productCount();

		// Check if index is out of bounds
		// It's an unsigned integer, so we only need to check the right side bounds
		if (index >= size) {
			throw std::out_of_range("Index is out of range.");

			return _products[0]; // We will return the first element, but will throw an error
		}

		return _products[index];
	}

	crow::json::wvalue toJSON()
	{
		crow::json::wvalue jsonObject;
		jsonObject["id"] = this->_id;
		jsonObject["user"] = this->_userID;
		jsonObject["products"] = this->_products;
		return jsonObject;
	}
};

// Function Definitions
std::string loadFile(response& res, std::string _folder, std::string _name);
bool isAuthorized(ID userID, const request& req);
bool initDB(sqlite3* db);

// Main Function
int main()
{
	crow::SimpleApp app;
	map<ID, Cart> carts;
	sqlite3* db;

	#ifdef DEBUG
		carts.insert_or_assign(0, Cart(0, 0));
		carts.at(0).products.push_back("test product");
	#endif

	// Let's initialize the database before accepting any requests

	initDB(db);


	// DEBUG
	sqlite3_close(db);



	CROW_ROUTE(app, "/<int>") // Products Page
		([](const request& req, response& res, int userID){
			res.set_header("Content-Type", "text/html");

			// Load the html file
			string indexhtml = loadFile(res, "", "index.html");

			//indexhtml = replaceItemTemplates(indexhtml);
			

			// Write the final html to the result body
			res.write(indexhtml);
			res.end();
		});

	CROW_ROUTE(app, "/profile/<int>")
	.methods(HTTPMethod::GET)
		([](const request& req, response& res, int profileID){
			std::string pidString = std::to_string(profileID);

			// Temporary Redirect
			res.code = 307;

			// Set the redirection URL
			std::stringstream profileURL;

			// THE PROFILE MODULE DOES NOT HAVE A DECIDED URL FOR PROFILES AT THE TIME OF WRITING
			// Once the module does have a decided url endpoint, the below line MUST be updated to point to the correct url

			profileURL << PROFILE << "/WE_DONT_KNOW_YET/" << pidString << "/";
			res.set_header("Location", profileURL.str());
			
			// If the browser or client does not redirect, they will see a message showing where we are trying to redirect to
			std::stringstream message;
			message << "Redirecting to profile page for user: '" << pidString << "' at " << profileURL.str();
			res.write(message.str());

			res.end();
		});

  
	CROW_ROUTE(app, "/cart.js") 
		([](const request& req, response& res){
			res.set_header("Content-Type", "text/javascript");
			
			res.write(loadFile(res, "js/", "cart.js"));
			
			res.end();
		});

	CROW_ROUTE(app, "/cart/<int>") 
		([&carts](response& res, int user){
			if(carts.contains(user))
			{
				Cart cart = carts.at(user);
				res.write(cart.toJSON().dump());
				res.end();
				return;
			}
		
			res.code = 404;
			res.write("user not found");
			
			res.end();
		});


	app.port(23500).multithreaded().run();
	return 1;
}


// Function Definitions
string loadFile(response& res, std::string _folder, std::string _name) {
	std::string path = "/Shared/public/" + _folder + _name;

	ifstream file(path, ifstream::in);

	if (file) {
		ostringstream contents;
		contents << file.rdbuf();
		file.close();
		return contents.str();
	}
	else {
		res.set_header("Content-Type", "text/plain");
		res.code = 404;
		return path + "Not Found";
	}
}

// Check if a request is authorized to access the cart for some userID.
bool isAuthorized(ID userID, const request& req) {
	/// --- How to check for authorization ---
		// https://crowcpp.org/master/guides/auth/
	// Example of an authorization header entry
	/// Authorization: Basic bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==

	// Where...
	/// [Authorization:]  =   The header name
	/// [Basic ]   =   A prefix to the data, signifying the data is base64 encoded
	/// [bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==]    =    "myUniqueUsername:myPassword" (without "") encoded in base64

	// Get the full contents of the authorization header
	string authHeader = req.get_header_value("Authorization");

	// Remove the "Basic " keyword
	string base64 = authHeader.substr(6);

	// Decode the base64
	string rawAuth = crow::utility::base64decode(base64, base64.size());

	/// Now split the credentials into username and password
	unsigned int split = rawAuth.find(':');
	string username = rawAuth.substr(0, split);
	string pass = rawAuth.substr(split+1);


	// Now, verify that userID, username, and pass align with each other in the database
	return false;

}

// Initialize the database table
bool initDB(sqlite3* db) {

	string createTables[TABLES] = {
		"CREATE TABLE IF NOT EXISTS Users (id int NOT NULL UNIQUE, cartid int NOT NULL UNIQUE, name varchar(128) NOT NULL unique, passhash varchar(1024) NOT NULL);",
		"CREATE TABLE IF NOT EXISTS Carts (id int NOT NULL UNIQUE, userid int NOT NULL UNIQUE);",
		"CREATE TABLE IF NOT EXISTS Products (id int NOT NULL unique, cartid int NOT NULL, sellerid int NOT NULL, name varchar(128) NOT NULL, description varchar(4096), quantity int NOT NULL);"
	};

	// Open Database
	int exit = sqlite3_open(DB_PATH, &db);


	// Now attempt to run the defined queries
	for (int i=0;i<TABLES;i++) {
		// Run the query
		char* errorMsg;
		int exit = sqlite3_exec(db, createTables[i].c_str(), NULL, 0, &errorMsg);

		// Check if the table was created
		if (exit != SQLITE_OK) {
			cerr << "Error Creating Table: " << errorMsg << endl;
			sqlite3_free(errorMsg);
		} else {
			cout << "Table created/exists" << endl;
		}
	}

	return (exit == SQLITE_OK);

}
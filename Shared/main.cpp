#define CROW_MAIN

#include "headers/db.hpp"
#include "headers/cart.hpp"

#include <regex>
#include <iostream>
#include <string>

// Function Definitions
std::string loadFile(response& res, std::string _folder, std::string _name);
std::string replaceProductTemplates(std::string htmlString, Product newProd);
bool isAuthorized(ID userID, const request& req);

// Main Function
int main()
{
	crow::SimpleApp app;
	map<ID, Cart> carts;
	// Create and initialize the database
	DB db;

	

	#ifdef DEBUG
		carts.insert_or_assign(0, Cart(0, 0));
		carts.at(0).products.push_back("test product");
	#endif


	CROW_ROUTE(app, "/<int>") // Products Page
		([&db](const request& req, response& res, int userID){
			res.set_header("Content-Type", "text/html");

			// Load the html file
			string indexhtml = loadFile(res, "", "index.html");

			// This should actually be loaded directly into cart but this is for a demo/test
			vector<Product> prods = db.loadCartProducts(userID);

			// Replace item templates with items in the database
			for (int i=0;i<prods.size();i++) {
				indexhtml = replaceProductTemplates(indexhtml, prods[i]);
			}
			

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

std::string replaceProductTemplates(std::string htmlString, Product newProd) {
	const int templateSize = strlen(PRODUCT_TEMPLATE);

	// Find the location of the first occurance of the product template
	size_t loc = htmlString.find(PRODUCT_TEMPLATE);

	// Split the html in two, using the product template as a delimeter
	std::string before = htmlString.substr(0, loc);
	std::string after = htmlString.substr(loc + templateSize);


	// Build a new product html object
	stringstream code;
	code << "<li class=\"cart-item\">"
                "<img src=\"" << newProd.imgurl << "\" alt=\"" << newProd.name << "\">"
                "<div class=\"cart-item-details\">"
					"<div class=\"cart-item-details-small\">"
                    	"<h3 class=\"cart-item-title\">" << newProd.name << "</h3>"
                    	"<p class=\"cart-item-price\">$" << newProd.price << "</p>"
                    	"<p class=\"cart-item-quantity\">Quantity: " << newProd.quantity << "</p>"
                	"</div>"
					"<div class=\"cart-item-details-large\">"
						"<p class=\"cart-item-description\">" << newProd.description << "</p>"
					"</div>"
				"</div>"
                "<button class=\"cart-item-remove\">Remove</button>"
           "</li>";


	// Replace the template with the code block
	//htmlString.replace(htmlString.begin(), htmlString.begin()+loc, code.str());

	// Build the final result
	stringstream result;
	// Prefix the before string, then add our new code, then the product template again so we can do this al lagain, then the rest of the html
	result << before << code.str() << PRODUCT_TEMPLATE << after;

	return result.str();
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
	// WHat if we weren't given a header?

	// Remove the "Basic " keyword
	string base64 = authHeader.substr(6);
	// What if the data length is less than 6?

	// Decode the base64
	string rawAuth = crow::utility::base64decode(base64, base64.size());
	// What if we weren't given valid base64???

	/// Now split the credentials into username and password
	unsigned int split = rawAuth.find(':');
	string username = rawAuth.substr(0, split);
	string pass = rawAuth.substr(split+1);


	// Now, verify that userID, username, and pass align with each other in the database


	// Debug return false always
	return false;

}
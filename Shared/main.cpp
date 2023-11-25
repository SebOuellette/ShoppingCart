#define CROW_MAIN

#include "headers/db.hpp"
#include "headers/cart.hpp"
#include "headers/http.hpp"

#include <regex>
#include <iostream>
#include <string>


// Function Definitions
std::string loadFile(response& res, std::string _folder, std::string _name);
std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement);
std::string replaceProductTemplates(std::string htmlString, Product newProd);
bool isAuthorized(ID userID, const request& req);

// Main Function
int main()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	crow::SimpleApp app;
	map<ID, Cart> carts;
	// Create and initialize the database
	DB db;

	

	#ifdef DEBUG
		carts.insert_or_assign(0, Cart(0, 0));
		carts.at(0).products.push_back("test product");
	#endif

	CROW_ROUTE(app, "/api/upload/<int>") // upload product to cart
	.methods(HTTPMethod::POST, HTTPMethod::PUT)
        ([&db](const request& req, response& res,int userID){
            res.set_header("Content-Type", "text/html");

			// TODO Send request to product team asking for product info
			//   OR read request body for product info

			const crow::json::rvalue& parsed = crow::json::load(req.body);

			Product_s p;

			p.id = parsed["id"].i();
			p.sellerID = parsed["sellerid"].i();
			p.name = parsed["name"].s();
			p.description = parsed["description"].s();
			p.imgurl = parsed["imgurl"].s();
			p.price = parsed["cost"].d();
			p.quantity = 1;

            // Load the html file
            string indexhtml = loadFile(res, "", "index.html");
            bool worked=db.uploadCartProducts(userID,p);

            if(worked)
                res.code=200;
            else
                res.code=505;


            res.end();
        });

	CROW_ROUTE(app, "/api/remove/<int>/<int>") // upload product to cart
	.methods(HTTPMethod::DELETE, HTTPMethod::GET)
        ([&db](const request& req, response& res,int userID, int productID){

            // Load the html file
			std::stringstream query;
			query << "DELETE FROM Products WHERE cartid=(SELECT Carts.id FROM Carts WHERE userid=" << userID << ") AND Products.id=" << productID << ";";

            bool worked = db.run(query.str());

			// Redirect to the cart page
            res.code = 307;
			res.set_header("Location", std::string(CART) + "/" + to_string(userID));
		
			res.write("Redirecting to user cart");

            res.end();
        });

	CROW_ROUTE(app, "/<int>") // Products Page
		([&db](const request& req, response& res, int userID){

			res.set_header("Content-Type", "text/html");

			// Load the html file
			string indexhtml = loadFile(res, "", "index.html");
			indexhtml = replaceTemplates(indexhtml, USER_ID_TEMPLATE, std::to_string(userID));
			indexhtml = replaceTemplates(indexhtml, AD_TEMPLATE, AD);

			// This should actually be loaded directly into cart but this is for a demo/test
			vector<Product> prods = db.loadCartProducts(userID);
			float total = Cart::totalCost(prods);
			std::stringstream totalStream;
			totalStream << std::fixed << std::setprecision(2) << total;
			indexhtml = replaceTemplates(indexhtml, TOTAL_COST_TEMPLATE, totalStream.str());

			// Replace item templates with items in the database
			for (int i=0;i<prods.size();i++) {
				stringstream replacement;
				replacement << 
				"<li class=\"product\">"
                	"<img src=\"" << prods[i].imgurl << "\" alt=\"" << prods[i].name << "\">"
                	"<div class=\"product-details\">"
						"<div class=\"product-details-small\">"
                    		"<h3 class=\"product-title\">" << prods[i].name << "</h3>"
                    		"<p class=\"product-price\">$" << prods[i].price << "</p>"
                    		"<p class=\"product-quantity\">Quantity: " << prods[i].quantity << "</p>"
                		"</div>"
						"<div class=\"product-details-large\">"
							"<p class=\"product-description\">" << prods[i].description << "</p>"
						"</div>"
					"</div>"
                	"<button class=\"product-remove\"><a href=\"/api/remove/" << userID << "/" << prods[i].id << "\">Remove</a></button>"
           		"</li>" << PRODUCT_TEMPLATE;

				indexhtml = replaceTemplates(indexhtml, PRODUCT_TEMPLATE, replacement.str());
			}

			// TODO
			// Calculate total and replace the TOTAL_TEMPLATE
			
			// Write the final html to the result body
			res.write(indexhtml);

			res.end();
		});
			
	CROW_ROUTE(app, "/products") // Products Page
	([](const request& req, response& res){
		res.set_header("Content-Type", "text/html");
			
		res.write(loadFile(res, "", "index.html"));
			
		res.end();
	});

	CROW_ROUTE(app, "/home") // home Page
	([](const request& req, response& res){
		res.set_header("Content-Type", "text/html");
			
		res.write(loadFile(res, "", "home.html"));
			
		res.end();
	});

	CROW_ROUTE(app, "/checkout/<int>") // billing Page
	([&db](const request& req, response& res, int userID){
		/// Ensure the user is authorized
		// Read the authorization header
		std::string bearer = req.get_header_value("Authorization");
		std::stringstream ss(bearer);
		std::string tmp, token;
		ss >> tmp >> token;
		// Read the saved hashed password and ensure they match token=username:password

		/// Send cart info to the checkout module
		vector<Product> prods = db.loadCartProducts(userID);
		float total = Cart::totalCost(prods);

		crow::json::wvalue jsonObject;
		jsonObject["customerid"] = userID;
		jsonObject["total"] = total;
		jsonObject["products"] = prods;

		// Send data to checkout module
		res_t checkoutRes = HTTP::request(std::string(CHECKOUT) + "/" + to_string(userID), "POST\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"}, jsonObject.dump() + "\0");
		//res_t checkoutRes = http.request(std::string(CHECKOUT) + "/" + to_string(userID), "GET\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"});


		// We just assume the response is the link we redirect the user to
		std::string redirectURL = checkoutRes.body;
		if (redirectURL.length() == 0) {
			res.code = 404;
			res.write("No redirect page given.");
			res.end();
			return;
		}

		res.code = 307;
		res.set_header("Location", redirectURL);
		
		res.write("Redirecting to provided redirect URL '" + redirectURL + "'");
		//res.write(loadFile(res, "", "billing.html"));
			
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

std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement) {
	const int templateSize = strlen(templateStr);

	// Find the location of the first occurance of the product template
	size_t loc = htmlString.find(templateStr);

	// Split the html in two, using the product template as a delimeter
	std::string before = htmlString.substr(0, loc);
	std::string after = htmlString.substr(loc + templateSize);


	// Build the final result
	stringstream result;
	// Prefix the before string, then add our new code, then the rest of the html
	result << before << replacement << after;

	return result.str();
}

std::string replaceProductTemplates(std::string htmlString, Product newProd) {
	stringstream replacement;
	replacement << "<li class=\"product\">"
                "<img src=\"" << newProd.imgurl << "\" alt=\"" << newProd.name << "\">"
                "<div class=\"product-details\">"
					"<div class=\"product-details-small\">"
                    	"<h3 class=\"product-title\">" << newProd.name << "</h3>"
                    	"<p class=\"product-price\">$" << newProd.price << "</p>"
                    	"<p class=\"product-quantity\">Quantity: " << newProd.quantity << "</p>"
                	"</div>"
					"<div class=\"product-details-large\">"
						"<p class=\"product-description\">" << newProd.description << "</p>"
					"</div>"
				"</div>"
                "<button class=\"product-remove\">Remove</button>"
           "</li>" << PRODUCT_TEMPLATE;

	return replaceTemplates(htmlString, PRODUCT_TEMPLATE, replacement.str());
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
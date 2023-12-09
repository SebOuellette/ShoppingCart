#define CROW_MAIN

#include "headers/db.hpp"
#include "headers/cart.hpp"
#include "headers/http.hpp"

#include <regex>
#include <iostream>
#include <string>
#include <random>
#include <ctime>


// Function Definitions
string updateIndexTemplates(std::string indexhtml, vector<Product> &prods, ID userID, bool wishlist);
std::string loadFile(response& res, std::string _folder, std::string _name);
std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement);
res_t updateAnalysis(std::vector<Product> prods, std::string token, float checkoutTotal, ID userID);
bool isAuthorized(ID userID, const request& req);

// Main Function
int main()
{
	srand(time(NULL));

	curl_global_init(CURL_GLOBAL_DEFAULT);
	crow::SimpleApp app;
	map<ID, Cart> carts;
	// Create and initialize the database
	DB db;

	

	#ifdef DEBUG
		carts.insert_or_assign(0, Cart(0, 0));
		carts.at(0).products.push_back("test product");
	#endif

	CROW_ROUTE(app, "/api/upload/<string>") // upload product to cart
	.methods(HTTPMethod::POST, HTTPMethod::PUT)
        ([&db](const request& req, response& res,ID userID){
            res.set_header("Content-Type", "text/html");

		const crow::json::rvalue& parsed = crow::json::load(req.body);

		Product_s p;

		p.id = parsed["id"].s();
		p.sellerID = parsed["sellerid"].s();
		p.name = parsed["name"].s();
		p.description = parsed["description"].s();
		p.imgurl = parsed["imgurl"].s();
		p.price = parsed["cost"].d();
		p.quantity = 1;


            // Load the html file
            string indexhtml = loadFile(res, "", "index.html");
            
	 	bool worked = false;

		if (db.checkIfExists(userID, p.id, false)) {
			db.increaseProductQuantity(p.id, false);
			worked = true;
		} else {
	    		worked=db.uploadProducts(userID, p);
		}

            if(worked)
                res.code=200;
            else
                res.code=505;


            res.end();
        });

	CROW_ROUTE(app, "/api/wishlist/upload/<string>") // upload product to cart
	.methods(HTTPMethod::POST, HTTPMethod::PUT)
        ([&db](const request& req, response& res,ID userID){
            res.set_header("Content-Type", "text/html");

			const crow::json::rvalue& parsed = crow::json::load(req.body);

			Product_s p;

			p.id = parsed["id"].s();
			p.sellerID = parsed["sellerid"].s();
			p.name = parsed["name"].s();
			p.description = parsed["description"].s();
			p.imgurl = parsed["imgurl"].s();
			p.price = parsed["cost"].d();
			p.quantity = 1;

            // Load the html file
            string indexhtml = loadFile(res, "", "index.html");
             bool worked = false;

                if (db.checkIfExists(userID, p.id, true)) {
                        db.increaseProductQuantity(p.id, true);
                        worked = true;
                } else {
                        worked=db.uploadProducts(userID, p, true);
                }

            if(worked)
                res.code=200;
            else
                res.code=505;


            res.end();
        });

	CROW_ROUTE(app, "/api/remove/<string>/<string>") // remove product from cart
	.methods(HTTPMethod::DELETE, HTTPMethod::GET)
        ([&db](const request& req, response& res,ID userID, ID productID){

            // Load the html file
			std::stringstream query;
			query << "DELETE FROM Products WHERE cartid=(SELECT Carts.id FROM Carts WHERE userid=\"" << userID << "\") AND Products.id=\"" << productID << "\" LIMIT 1;";

            bool worked = db.run(query.str());

			// Redirect to the cart page
            res.code = 307;
			res.set_header("Location", std::string(CART) + "/" + userID);
		
			res.write("Redirecting to user cart");

            res.end();
        });

	CROW_ROUTE(app, "/api/decrease/<string>/<string>") // remove single product from cart
	.methods(HTTPMethod::DELETE, HTTPMethod::GET)
        ([&db](const request& req, response& res,ID userID, ID productID){

			db.decreaseProductQuantity(productID, userID, 0);
			// Redirect to the cart page
            res.code = 307;
			res.set_header("Location", std::string(CART) + "/" + userID);
		
			res.write("Redirecting to user cart");

            res.end();
        });


	CROW_ROUTE(app, "/api/increase/<string>/<string>") // add single product to cart
	.methods(HTTPMethod::DELETE, HTTPMethod::GET)
        ([&db](const request& req, response& res,ID userID, ID productID){

			db.increaseProductQuantity(productID, userID, 0);
			// Redirect to the cart page
            res.code = 307;
			res.set_header("Location", std::string(CART) + "/" + userID);
		
			res.write("Redirecting to user cart");

            res.end();
        });	

	CROW_ROUTE(app, "/api/wishlist/remove/<string>/<string>") // upload product to cart
	.methods(HTTPMethod::DELETE, HTTPMethod::GET)
        ([&db](const request& req, response& res,ID userID, ID productID){

            // Load the html file
			std::stringstream query;
			query << "DELETE FROM WantedProducts WHERE cartid=(SELECT Wishlists.id FROM Wishlists WHERE userid=\"" << userID << "\") AND WantedProducts.id=\"" << productID << "\" LIMIT 1;";

            bool worked = db.run(query.str());

			// Redirect to the cart page
            res.code = 307;
			res.set_header("Location", std::string(CART) + "/wishlist/" + userID);
		
			res.write("Redirecting to user cart");

            res.end();
        });

	CROW_ROUTE(app, "/wishlist/<string>") // Products Page
		([&db](const request& req, response& res, ID userID){
			// Remove any expired products from the list
			db.removeExpired(userID);

			// Then retrieve the updated list
			vector<Product> prods = db.loadProducts(userID, true);


			res.set_header("Content-Type", "text/html");
			
			// Load the html file
			string indexhtml = loadFile(res, "", "wishlist.html");
			indexhtml = updateIndexTemplates(indexhtml, prods, userID, true);

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

	CROW_ROUTE(app, "/checkout/<string>") // billing Page
	([&db](const request& req, response& res, ID userID){
		/// Ensure the user is authorized
		// Read the authorization header
		std::string bearer = req.get_header_value("Authorization");
		std::stringstream ss(bearer);
		std::string tmp, token;
		ss >> tmp >> token;
		// Read the saved hashed password and ensure they match token=username:password

		/// Send cart info to the checkout module
		vector<Product> prods = db.loadProducts(userID);
		float total = Cart::totalCost(prods);

		crow::json::wvalue jsonObject;
		jsonObject["customerid"] = userID;
		jsonObject["total"] = total;
		jsonObject["products"] = prods;

		// Send data to checkout module
		res_t checkoutRes = HTTP::request(std::string(CHECKOUT) + "/api/cartinfo", "POST\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"}, jsonObject.dump() + "\0");
		//res_t checkoutRes = http.request(std::string(CHECKOUT) + "/" + to_string(userID), "GET\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"});

		res_t analysisRes = updateAnalysis(prods, token, total, userID);

		// Ad module also wants this data, so send it to them
		res_t AdRes = HTTP::request(std::string(AD) + "/api/parsejson", "POST\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"}, jsonObject.dump() + "\0");


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

	CROW_ROUTE(app, "/profile")
	.methods(HTTPMethod::GET)
		([](const request& req, response& res){
			// Temporary Redirect
			res.code = 307;

			// Set the redirection URL
			std::stringstream profileURL;

			profileURL << PROFILE << "/home/MenuPage";
			res.set_header("Location", profileURL.str());
			
			// If the browser or client does not redirect, they will see a message showing where we are trying to redirect to
			std::stringstream message;
			message << "Redirecting to profile page for user at " << profileURL.str();
			res.write(message.str());

			res.end();
		});

  
	CROW_ROUTE(app, "/cart.js") 
		([](const request& req, response& res){
			res.set_header("Content-Type", "text/javascript");
			
			res.write(loadFile(res, "js/", "cart.js"));
			
			res.end();
		});

	// CROW_ROUTE(app, "/cart/<string>") 
	// 	([&carts](response& res, int user){
	// 		if(carts.contains(user))
	// 		{
	// 			Cart cart = carts.at(user);
	// 			res.write(cart.toJSON().dump());
	// 			res.end();
	// 			return;
	// 		}
		
	// 		res.code = 404;
	// 		res.write("user not found");
			
	// 		res.end();
	// 	});



	CROW_ROUTE(app, "/<string>") // Products Page
		([&db](const request& req, response& res, ID userID){
			// Remove any expired products from the list
			db.removeExpired(userID);

			// Then retrieve the updated list
			vector<Product> prods = db.loadProducts(userID, false);


			res.set_header("Content-Type", "text/html");
			
			// Load the html file
			string indexhtml = loadFile(res, "", "index.html");
			indexhtml = updateIndexTemplates(indexhtml, prods, userID, false);

			// TODO
			// Calculate total and replace the TOTAL_TEMPLATE
			
			// Write the final html to the result body
			res.write(indexhtml);

			res.end();
		});
		
	app.port(23500).multithreaded().run();
	return 1;
}

// Update the analysis module with the product information
// This function is called during the checkout process
res_t updateAnalysis(std::vector<Product> prods, std::string token, float checkoutTotal, ID userID) {
	// Calculate the current time
	std::time_t ctime = std::time(NULL);
	// Format the time as string
	char buffer[30];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S\0", std::localtime(&ctime));
	std::string timestr(buffer);


	// Create the main json object
	crow::json::wvalue mainObject;
	mainObject["Transaction_ID"] = std::to_string((unsigned short int)rand());
	mainObject["User_ID"] = userID;
	mainObject["Order_Value"] = checkoutTotal;
	mainObject["date"] = timestr;
	
	// Build the list of products
	crow::json::wvalue productList;
	for (int i=0;i<prods.size();i++) {
		crow::json::wvalue prodObject;

		prodObject["Product_ID"] = prods[i].id;
		prodObject["Product_Price"] = prods[i].price;
		prodObject["Product_Quantity"] = prods[i].quantity;
		productList[i] = std::move(prodObject);
	}

	// Add the list of products to the root json
	mainObject["Details_Products"] = std::move(productList);



	return HTTP::request(std::string(ANALYSIS) + "/Group1/DatabaseController/POST/minhnguyen/Connhenbeo1/group1/2", "POST\0", {"Authorization: Bearer " + token + "\0", "Context-type: application/json\0"}, mainObject.dump() + "\0");
}

string updateIndexTemplates(std::string indexhtml, vector<Product> &prods, ID userID, bool wishlist) {
	if (!wishlist) 
		indexhtml = replaceTemplates(indexhtml, USER_ID_TEMPLATE, userID);
	indexhtml = replaceTemplates(indexhtml, USER_ID_TEMPLATE, userID); // Update the second user ID template
	indexhtml = replaceTemplates(indexhtml, AD_TEMPLATE, AD);
	indexhtml = replaceTemplates(indexhtml, HOME_LINK_TEMPLATE, HOME);
	//indexhtml = replaceTemplates(indexhtml, PRODUCTS_LINK_TEMPLATE, HOME); // PRODUCT is the api link

	// This should actually be loaded directly into cart but this is for a demo/test
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
			"<button class=\"product-remove\"><a href=\"/api/decrease/"<<userID<<"/"<<prods[i].id<<"\">-</a></button>"
			"<button class=\"product-remove\"><a href=\"/api/increase/"<<userID<<"/"<<prods[i].id<<"\">+</a></button>"
			"<button class=\"product-remove\"><a href=\"/api/"<<(wishlist?"wishlist/":"")<<"remove/" << userID << "/" << prods[i].id << "\">Remove</a></button>"
		"</li>" << PRODUCT_TEMPLATE;

		indexhtml = replaceTemplates(indexhtml, PRODUCT_TEMPLATE, replacement.str());
	}

	return indexhtml;
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
	if (loc <= templateSize) {
		return htmlString;
	}

	// Split the html in two, using the product template as a delimeter
	std::string before = htmlString.substr(0, loc);
	std::string after = htmlString.substr(loc + templateSize);


	// Build the final result
	stringstream result;
	// Prefix the before string, then add our new code, then the rest of the html
	result << before << replacement << after;

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

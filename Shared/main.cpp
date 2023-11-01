#define CROW_MAIN

#include "crow_all.h"
#include <regex>
#include <iostream>
using namespace std;
using namespace crow;

#define PROFILE "http://localhost:8081"

std::string loadFile(response& res, std::string _folder, std::string _name);



class Cart
{
	public:
	int id;
	int user;
	vector<string> products;

	Cart(int id, int userID)
	{
		this->id = id;
		this->user = userID;
	}

	crow::json::wvalue toJSON()
	{
		crow::json::wvalue jsonObject;
		jsonObject["id"] = this->id;
		jsonObject["user"] = this->user;
		jsonObject["products"] = this->products;
		return jsonObject;
	}
};


map<int, Cart> carts;

int main()
{
	crow::SimpleApp app;

	#ifdef DEBUG
		carts.insert_or_assign(0, Cart(0, 0));
		carts.at(0).products.push_back("test product");
	#endif



	CROW_ROUTE(app, "/") // Products Page
		([](const request& req, response& res){
			res.set_header("Content-Type", "text/html");
			
			res.write(loadFile(res, "", "index.html"));
			
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
		([](response& res, int user){
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

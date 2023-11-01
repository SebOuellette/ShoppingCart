#define CROW_MAIN

#include "crow_all.h"
#include <regex>
#include <iostream>
using namespace std;
using namespace crow;

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

	carts.insert_or_assign(0, Cart(0, 0));
	carts.at(0).products.push_back("test product");




	CROW_ROUTE(app, "/") // Products Page
		([](const request& req, response& res){
			res.set_header("Content-Type", "text/html");
			
			res.write(loadFile(res, "", "index.html"));
			
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

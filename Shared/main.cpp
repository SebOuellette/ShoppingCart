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
};


map<int, Cart> carts;

int main()
{
	crow::SimpleApp app;


	CROW_ROUTE(app, "/") // Products Page
		([](const request& req, response& res){
			res.set_header("Content-Type", "text/html");
			
			res.write(loadFile(res, "", "index.html"));
			
			res.end();
		});

	app.port(23500).multithreaded().run();

	CROW_ROUTE(app, "/cart/<int>") 
		([](response& res, int user){
			res.set_header("Content-Type", "text/html");
			Cart cart = carts.at(user);
			res.write(to_string(cart.id));
			res.end();
		});

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

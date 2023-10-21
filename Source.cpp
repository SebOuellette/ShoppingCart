#include <iostream>
#include "headers/crow_all.h"

int main() {
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")
		([]() {
		return "Hello, World!";
		});

	return 0;
}
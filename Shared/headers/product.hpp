#ifndef PRODUCT_HPP
#define PRODUCT_HPP

#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Product {
	ID id;
	ID sellerID;
	char name[NAME_LENGTH];
	char description[DESCRIPTION_LENGTH];
    char imgurl[URL_LENGTH];
	double price;
	unsigned int quantity = 1;

	std::chrono::system_clock::time_point timeAdded;		//variable to record when product was added to cart

	_Product() : timeAdded(std::chrono::system_clock::now())	{}		//marks time when product was added to cart

	bool isExpired() {
		auto currentTime = std::chrono::system_clock::now();
		auto difference = currentTime - timeAdded;

		auto months = std::chrono::duration_cast<std::chrono::months>(difference);

		if (months.count() < 2)	{
			return 0;
		}
		else {
			return 1;
		}

	}


	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const { 
		return id;
	}
} Product;


#endif
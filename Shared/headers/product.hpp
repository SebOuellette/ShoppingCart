#ifndef PRODUCT_HPP
#define PRODUCT_HPP

#include "defines.hpp"

typedef struct _Product {
	ID id;
	ID sellerID;
	char name[NAME_LENGTH];
	char description[DESCRIPTION_LENGTH];
	double price;
	unsigned int quantity = 1;

	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const { 
		return id;
	}
} Product;


#endif
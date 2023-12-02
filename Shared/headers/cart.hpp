#ifndef CART_HPP
#define CART_HPP

#include "db.hpp"
#include "defines.hpp"
#include "product.hpp"
#include <vector>

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

		//_products = _db.loadCartProducts(userID);
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

	vector<Product> getProductsCopy() {
		vector<Product> copy = _products;

		return copy;
	}

	void addProduct(Product newP) {
		this->_products.push_back(newP);
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

	// Calculate the total cost of products
	static float totalCost(vector<Product> products) {
		float total = 0;
		
		for (Product p : products) {
			total += p.price;
		}

		return total;
	}

	// Return a reference to the product at index index
	// Product& at(ID index) {
	// 	int size = this->productCount();

	// 	// Check if index is out of bounds
	// 	// It's an unsigned integer, so we only need to check the right side bounds (it already can't be negative)
	// 	if (index >= size) {
	// 		throw std::out_of_range("Index is out of range.");

	// 		return _products[0]; // We will return the first element, but will throw an error
	// 	}

	// 	return _products[index];
	// }

	crow::json::wvalue toJSON()
	{
		crow::json::wvalue jsonObject;
		jsonObject["cartid"] = this->_id;
		jsonObject["customerid"] = this->_userID;
		jsonObject["products"] = this->_products;
		return jsonObject;
	}
};




#endif
#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "crow_all.h"
using namespace crow;
using namespace std;


// Program Defines
#define TABLES 3
#define DB_PATH "../cart.db" // will create cart.db in the shared folder


// Module IP Addresses
#define PROFILE "http://localhost:8081"

// Product Defines
#define NAME_LENGTH 128
#define PASSWD_HASH_LENGTH 1024
#define DESCRIPTION_LENGTH 4096
#define URL_LENGTH 512

// ID variable type is an unsigned 64-bit integer
typedef unsigned long long int ID;

// HTML templates. Used for adding dynamically generated content to the html pages from server-side
#define PRODUCT_TEMPLATE "<!--[[NEXT-CART-ITEM]]-->"

#endif
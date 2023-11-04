# Shopping Cart Module

This is the shopping cart module for CSCN73030 Lab 2. 

## Documentation

### Legend
`<hello>` -- Text surrounded by <> is a variable. Do not include <> when using the API.<br>
`?foobar?` -- Text surrounded by ?? is content that is to be decided later in development. 

### Link to Profile Module
#### Request
`GET`  `/profile/<profile_id>`

### Link to ad Module
#### Request
`GET`  `/cart/<userID>`

#### Response
```
307: Temporary Redirect
Location: http://<profile_module_ADDR>/?profile_endpoint?/<profile_id>

Redirecting to profile page for user: '<profile_id>' at http://<profile_module_ADDR>/?profile_endpoint?/<profile_id>
```



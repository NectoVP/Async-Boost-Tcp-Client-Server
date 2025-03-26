# Async multithreaded server for food restaurant 

# Server

Build and run build/TcpServer 

you can modify main.cpp to specify port, ip and number of threads to use

Note that, all info is stored in RAM and will be removed after restart
Note that, Service Layer is independent of tcp and can be used without opening connections (see examples in Server_tests.cpp)

## Client

Example code for client is written in /tests/Net_tests.cpp

Works exactly the same, you specify ip, port, target, HTTP request type and index

Index is an id of the object in json_files provided in test directory.
Feel free to change this logic

## Supported methods

- get /get_description (that retrieves all info about products)
- post /buy (place product in cart)
with json, containing itemId, itemCount and sessionId
(see example in /tests/test_buy.json)
- delete /remove (remove product from the cart) 
with json, containing itemId, itemCount and sessionId
(see example in /tests/test_remove.json) 
- post /make_order (to make your order)
with json, containing total sum for the order, and SessionId
(see example in /tests/test_make_order.json)

### Tests

When running tests, make sure to restart manually server for each run of the tests


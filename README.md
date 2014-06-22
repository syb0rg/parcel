# parcel [<img align="right"  src="https://travis-ci.org/syb0rg/parcel.svg?branch=master"/>](https://travis-ci.org/syb0rg/parcel)

An efficient and functional JSON parsing library written in C.  parcel was designed to be small and portable for seamless integration will resource-constrained systems.

### Features

 - Backwards compatable with the C99 standard.
 
 - No external dependencies.
 
 - Proved to be faster then other JSON parsing libraries in benchmarking tests.
 
 - Follows strict JSON syntax
 
 - Includes Doxygen commends for generating documentation

### Design

parcel splits a JSON file into tokens.  Consider the following example of JSON data:

	{
	     "firstName": "John",
	     "lastName": "Smith",
	     "address": {
	         "streetAddress": "21 2nd Street",
	         "city": "New York",
	         "state": "NY",
	         "postalCode": 10021
	     },
	     "phoneNumbers": [
	         "212 555-1234",
	         "646 555-4567"
	     ]
	 }
	 
 parcel will split that data into the following tokens based on [the JSON values](http://www.w3schools.com/json/json_syntax.asp):
 
 - **Objects**: `{
	     "firstName": "John",
	     "lastName": "Smith",
	     "address": {
	         "streetAddress": "21 2nd Street",
	         "city": "New York",
	         "state": "NY",
	         "postalCode": 10021
	     },
	     "phoneNumbers": [
	         "212 555-1234",
	         "646 555-4567"
	     ]
	 }` (the whole object), `{
	         "streetAddress": "21 2nd Street",
	         "city": "New York",
	         "state": "NY",
	         "postalCode": 10021
	     }`

 - **Arrays**: `[
	         "212 555-1234",
	         "646 555-4567"
	     ]`

 - **Strings**: `"firstName"`, `"John"`, `"lastName"`, `"Smith"`, `"address"`, `"streetAddress"`, `"21 2nd Street"`, `"city"`, `"New York"`, `"state"`, `"NY"`, `"postalCode"`, `"phoneNumbers"`, `"212 555-1234"`, `"646 555-4567"`
 
 - **Primitives** (a number, boolean, or NULL): `10021`
 
However, it is important to note that parcel only points to the token boundaries.  Taking the example from above:
 
 - **Objects**: `[0, 286]`, `[76, 203]`
 
 - **Arrays**: `[227, 283]`
 
And so on for Strings and Primitives.  Besides that token boundaries, parcel tokens for more complex data types (objects and arrays), also contain the number of child items, so you can easily follow the hierarchy.
 
This approach provides enough information for parsing any JSON data and makes it possible to use zero-copy techniques; making the parsing of JSON data a lot faster and more simple.
 
 ---
 
### TODO
 
 - Finish documentation.
 
 - Add a more flexable JSON item retrieval function.
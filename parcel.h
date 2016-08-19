/**
 * @file PARCEL_H
 * @brief parcel structures, enums, and relevent function prototypes.
 */
#ifndef PARCEL_H
#define PARCEL_H

#include <stdint.h>

/**
 * JSON type identifier. Basic types are:
 * 	- Object
 * 	- Array
 * 	- String
 * 	- Other primitive: number, boolean (true/false) or null
 */
typedef enum { PARCEL_PRIMITIVE, PARCEL_OBJECT, PARCEL_ARRAY, PARCEL_STRING } ParcelType;

typedef enum
{
	PARCEL_ERROR_NOMEM = -1, // Not enough tokens were provided
	PARCEL_ERROR_INVAL = -2, // Invalid character inside JSON string
	PARCEL_ERROR_PART = -3, // The string is not a full JSON packet, more bytes expected
	PARCEL_SUCCESS = 0 // Everthing is fine
} ParcelError;

/**
 * JSON token description.
 * @var type The type (object, array, string etc.)
 * @var start The start position in JSON data string.
 * @var end The end position in JSON data string.
 */
typedef struct
{
	ParcelType type;
	intmax_t start;
	intmax_t end;
	intmax_t size;
	intmax_t parent;
} __attribute__((__packed__)) ParcelToken;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 * @var pos Contains the offset in the JSON string.
 * @var toknext Contains the next token to allocate.
 * @var toksuper Contains the superior token node (e.g parent object or array).
 */
typedef struct
{
	intmax_t pos;
	intmax_t toknext;
	intmax_t toksuper;
} __attribute__((__packed__)) ParcelParser;

/**
 * Initializes the JSON parser over an array of tokens
 */
void parcel_initParser(ParcelParser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
ParcelError parcel_parse(ParcelParser *parser, const char *js, ParcelToken *tokens, unsigned int tokenNum);

/**
 * Fetch data from the given item in the supplied JSON file.
 */
const char* parcel_getItemFromJSON(const char *json, const char *item);


#endif // _parcel_H_

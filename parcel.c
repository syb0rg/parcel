/**
 * @file json.c
 * @brief JSON Parser
 */
#include <string.h>
#include "parcel.h"

#define TOKEN_NUMBER 100

/**
 * @fn const char* getItemFromJSON(const char *json, const char *item)
 * @brief Retrives the specified item data from the given JSON
 * @param json The JSON data to be parsed
 * @param item The item in which the data is to be fetched
 * @return The data contained in the first occurance of item
 */
const char* parcel_getItemFromJSON(const char *json, const char *item)
{
	if (!json || !item) return NULL;
	JsonParser p;
	parcel_initParser(&p);
	JsonToken tokens[TOKEN_NUMBER];
	memset(&tokens, 0, sizeof(tokens));
	int err = parcel_parse(&p, json, tokens, TOKEN_NUMBER);
	if (err) return NULL;
	for(size_t i = 0; i < TOKEN_NUMBER; ++i)
	{
		JsonToken *key = &tokens[i];
		if ((key->end - key->start) < 0) return NULL;
		if (!memcmp("", &json[key->start], (size_t) (key->end - key->start))) return NULL;
		if (!memcmp(item, &json[key->start], (size_t) (key->end - key->start)))
		{
			++key;
			return strndup(&json[key->start], (size_t) (key->end - key->start));
		}
	}
	return NULL;
}

/**
 * @fn static JsonToken *parcel_allocJsonToken(JsonParser *parser, JsonToken *tokens, size_t tokenNum)
 * @brief Allocates a fresh unused token from the token pull.
 * @param parser An initialized JsonParser structure
 * @param tokens
 * @param tokenNum
 */
static JsonToken* parcel_initToken(JsonParser *parser, JsonToken *tokens, intmax_t tokenNum)
{
	if (parser->toknext >= tokenNum) return NULL;
	JsonToken *tok = &tokens[parser->toknext++];
	tok->start = tok->end = -1;
	tok->size = 0;
	tok->parent = -1;
	return tok;
}

/**
 * @fn static void parcel_fillToken(JsonToken *token, JsonType type, int start, int end)
 * @brief Fills token type and boundaries.
 * @param token
 * @param type
 * @param start
 * @param end
 */
static void parcel_fillToken(JsonToken *token, JsonType type, intmax_t start, intmax_t end)
{
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * @fn static JsonError parcel_parsePrimitive(JsonParser *parser, const char *js, JsonToken *tokens, size_t tokenNum)
 * @brief Fills next available token with JSON primitive.
 * @param parser An initialized JsonParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
static JsonError parcel_parsePrimitive(JsonParser *parser, const char *js, JsonToken *tokens, intmax_t tokenNum)
{
	JsonToken *token;
	intmax_t start = parser->pos;

	for (; js[parser->pos] != '\0'; parser->pos++)
	{
		switch (js[parser->pos])
		{
			case ':':
			case '\t':
			case '\r':
			case '\n':
			case ' ':
			case ',':
			case ']':
			case '}':
				goto found;
		}
		if (js[parser->pos] < 32 || js[parser->pos] >= 127)
		{
			parser->pos = start;
			return JSON_ERROR_INVAL;
		}
	}
	parser->pos = start;
	return JSON_ERROR_PART;

found:
	token = parcel_initToken(parser, tokens, tokenNum);
	if (!token)
	{
		parser->pos = start;
		return JSON_ERROR_NOMEM;
	}
	parcel_fillToken(token, JSON_PRIMITIVE, start, parser->pos);
	token->parent = parser->toksuper;
	parser->pos--;
	return JSON_SUCCESS;
}

/**
 * @fn static JsonError parcel_parseString(JsonParser *parser, const char *js, JsonToken *tokens, size_t tokenNum)
 * @brief Fills next token with JSON string.
 * @param parser An initialized JsonParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
static JsonError parcel_parseString(JsonParser *parser, const char *js, JsonToken *tokens, intmax_t tokenNum)
{
	JsonToken *token;
	intmax_t start = parser->pos;

	parser->pos++;

	// Skip starting quote
	for (; js[parser->pos] != '\0'; parser->pos++)
	{
		char c = js[parser->pos];

		// Quote: end of string
		if (c == '\"')
		{
			token = parcel_initToken(parser, tokens, tokenNum);
			if (!token)
			{
				parser->pos = start;
				return JSON_ERROR_NOMEM;
			}
			parcel_fillToken(token, JSON_STRING, start+1, parser->pos);
			token->parent = parser->toksuper;
			return JSON_SUCCESS;
		}

		// Backslash: Quoted symbol expected
		if (c == '\\')
		{
			parser->pos++;
			switch (js[parser->pos])
			{
				// Allowed escaped symbols
				case '\"':
				case '/':
				case '\\':
				case 'b':
				case 'f':
				case 'r':
				case 'n':
				case 't':
					break;
				// Allows escaped symbol \uXXXX
				case 'u':
					/// \todo handle JSON unescaped symbol \\uXXXX
					break;
				// Unexpected symbol
				default:
					parser->pos = start;
					return JSON_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return JSON_ERROR_PART;
}

/**
 * @fn JsonError parcel_parse(JsonParser *parser, const char *js, JsonToken *tokens, unsigned int tokenNum)
 * @brief Parse JSON string and fill tokens.
 * @param parser An initialized JsonParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
JsonError parcel_parse(JsonParser *parser, const char *js, JsonToken *tokens, unsigned int tokenNum)
{
	JsonError r;
	JsonToken *token;

	for (; js[parser->pos] != '\0'; parser->pos++)
	{
		char c = js[parser->pos];;
		JsonType type;

		switch (c)
		{
			case '{':
			case '[':
				token = parcel_initToken(parser, tokens, tokenNum);
				if (!token) return JSON_ERROR_NOMEM;
				if (parser->toksuper != -1)
				{
					tokens[parser->toksuper].size++;
					token->parent = parser->toksuper;
				}
				token->type = (c == '{' ? JSON_OBJECT : JSON_ARRAY);
				token->start = parser->pos;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}':
			case ']':
				type = (c == '}' ? JSON_OBJECT : JSON_ARRAY);
				if (parser->toknext < 1) return JSON_ERROR_INVAL;
				token = &tokens[parser->toknext - 1];
				for (;;)
				{
					if (token->start != -1 && token->end == -1)
					{
						if (token->type != type) return JSON_ERROR_INVAL;
						token->end = parser->pos + 1;
						parser->toksuper = token->parent;
						break;
					}
					if (token->parent == -1) break;
					token = &tokens[token->parent];
				}
				break;
			case '\"':
				r = parcel_parseString(parser, js, tokens, tokenNum);
				if (r < 0) return r;
				if (parser->toksuper != -1) tokens[parser->toksuper].size++;
				break;
			case '\t':
			case '\r':
			case '\n':
			case ':':
			case ',':
			case ' ':
				break;
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 't':
			case 'f':
			case 'n':
				r = parcel_parsePrimitive(parser, js, tokens, tokenNum);
				if (r < 0) return r;
				if (parser->toksuper != -1) tokens[parser->toksuper].size++;
				break;
			default:
				return JSON_ERROR_INVAL;

		}
	}

	for (intmax_t i = parser->toknext - 1; i >= 0; i--)
	{
		// Unmatched opened object or array
		if (tokens[i].start != -1 && tokens[i].end == -1) return JSON_ERROR_PART;
	}

	return JSON_SUCCESS;
}

/**
 * @fn void parcel_initParser(JsonParser *parser)
 * @brief Creates a new parser based over a given buffer with an array of tokens available.
 * @param parser A JsonParser structure to initialize
 */
void parcel_initParser(JsonParser *parser)
{
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}


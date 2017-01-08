#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
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
	ParcelParser p;
	parcel_initParser(&p);
	ParcelToken tokens[TOKEN_NUMBER];
	memset(&tokens, 0, sizeof(tokens));
	int err = parcel_parse(&p, json, tokens, TOKEN_NUMBER);
	if (err) return NULL;
	for(size_t i = 0; i < TOKEN_NUMBER; ++i)
	{
		ParcelToken *key = &tokens[i];
		if ((key->end - key->start) < 0) return NULL;
		if (!memcmp("", &json[key->start], (size_t) (key->end - key->start))) return NULL;
		if (!memcmp(item, &json[key->start], (size_t) (key->end - key->start)))
		{
			++key;
			return strdup(&json[key->start]);
		}
	}
	return NULL;
}

/**
 * @fn static ParcelToken *parcel_allocParcelToken(ParcelParser *parser, ParcelToken *tokens, size_t tokenNum)
 * @brief Allocates a fresh unused token from the token pull.
 * @param parser An initialized ParcelParser structure
 * @param tokens
 * @param tokenNum
 */
static ParcelToken* parcel_initToken(ParcelParser *parser, ParcelToken *tokens, intmax_t tokenNum)
{
	if (parser->toknext >= tokenNum) return NULL;
	ParcelToken *tok = &tokens[++parser->toknext];
	tok->start = tok->end = -1;
	tok->size = 0;
	tok->parent = -1;
	return tok;
}

/**
 * @fn static void parcel_fillToken(ParcelToken *token, ParcelType type, int start, int end)
 * @brief Fills token type and boundaries.
 * @param token
 * @param type
 * @param start
 * @param end
 */
static void parcel_fillToken(ParcelToken *token, ParcelType type, intmax_t start, intmax_t end)
{
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * @fn static ParcelError parcel_parsePrimitive(ParcelParser *parser, const char *js, ParcelToken *tokens, size_t tokenNum)
 * @brief Fills next available token with JSON primitive.
 * @param parser An initialized ParcelParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
static ParcelError parcel_parsePrimitive(ParcelParser *parser, const char *js, ParcelToken *tokens, intmax_t tokenNum)
{
	ParcelToken *token;
	intmax_t start = parser->pos;

	for (; js[parser->pos] != '\0'; ++parser->pos)
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
			return PARCEL_ERROR_INVAL;
		}
	}
	parser->pos = start;
	return PARCEL_ERROR_PART;

found:
	token = parcel_initToken(parser, tokens, tokenNum);
	if (!token)
	{
		parser->pos = start;
		return PARCEL_ERROR_NOMEM;
	}
	parcel_fillToken(token, PARCEL_PRIMITIVE, start, parser->pos);
	token->parent = parser->toksuper;
	--parser->pos;
	return PARCEL_SUCCESS;
}

/**
 * @fn static ParcelError parcel_parseString(ParcelParser *parser, const char *js, ParcelToken *tokens, size_t tokenNum)
 * @brief Fills next token with JSON string.
 * @param parser An initialized ParcelParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
static ParcelError parcel_parseString(ParcelParser *parser, const char *js, ParcelToken *tokens, intmax_t tokenNum)
{
	ParcelToken *token;
	intmax_t start = parser->pos;

	++parser->pos;

	// Skip starting quote
	for (; js[parser->pos] != '\0'; ++parser->pos)
	{
		char c = js[parser->pos];

		// Quote: end of string
		if (c == '\"')
		{
			token = parcel_initToken(parser, tokens, tokenNum);
			if (!token)
			{
				parser->pos = start;
				return PARCEL_ERROR_NOMEM;
			}
			parcel_fillToken(token, PARCEL_STRING, start+1, parser->pos);
			token->parent = parser->toksuper;
			return PARCEL_SUCCESS;
		}

		// Backslash: Quoted symbol expected
		if (c == '\\')
		{
			++parser->pos;
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
					return PARCEL_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return PARCEL_ERROR_PART;
}

/**
 * @fn ParcelError parcel_parse(ParcelParser *parser, const char *js, ParcelToken *tokens, unsigned int tokenNum)
 * @brief Parse JSON string and fill tokens.
 * @param parser An initialized ParcelParser structure
 * @param js
 * @param tokens
 * @param tokenNum
 */
ParcelError parcel_parse(ParcelParser *parser, const char *js, ParcelToken *tokens, unsigned int tokenNum)
{
	ParcelError r;
	ParcelToken *token;

	for (; js[parser->pos] != '\0'; ++parser->pos)
	{
		char c = js[parser->pos];;
		ParcelType type;

		switch (c)
		{
			case '{':
			case '[':
				token = parcel_initToken(parser, tokens, tokenNum);
				if (!token) return PARCEL_ERROR_NOMEM;
				if (parser->toksuper != -1)
				{
					++tokens[parser->toksuper].size;
					token->parent = parser->toksuper;
				}
				token->type = (c == '{' ? PARCEL_OBJECT : PARCEL_ARRAY);
				token->start = parser->pos;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}':
			case ']':
				type = (c == '}' ? PARCEL_OBJECT : PARCEL_ARRAY);
				if (parser->toknext < 1) return PARCEL_ERROR_INVAL;
				token = &tokens[parser->toknext - 1];
				for (;;)
				{
					if (token->start != -1 && token->end == -1)
					{
						if (token->type != type) return PARCEL_ERROR_INVAL;
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
				if (parser->toksuper != -1) ++tokens[parser->toksuper].size;
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
				if (parser->toksuper != -1) ++tokens[parser->toksuper].size;
				break;
			default:
				return PARCEL_ERROR_INVAL;

		}
	}

	for (intmax_t i = parser->toknext - 1; i >= 0; --i)
	{
		// Unmatched opened object or array
		if (tokens[i].start != -1 && tokens[i].end == -1) return PARCEL_ERROR_PART;
	}

	return PARCEL_SUCCESS;
}

/**
 * @fn void parcel_initParser(ParcelParser *parser)
 * @brief Creates a new parser based over a given buffer with an array of tokens available.
 * @param parser A ParcelParser structure to initialize
 */
void parcel_initParser(ParcelParser *parser)
{
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}

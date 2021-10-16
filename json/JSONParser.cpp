//
//  JSONParser.cpp
//  JSONSyntaxChecker
//
//  Created by William Woody on 11/16/20.
//

#include <stdio.h>
#include <stdarg.h>
#include "JSON.h"

/****************************************************************************/
/*																			*/
/*	Internal Constants														*/
/*																			*/
/****************************************************************************/

/*	JSONParser::JSONParser
 *
 *		Parser
 */

JSONParser::JSONParser()
{
}

/*	JSONParser::~JSONParser
 *
 *		Parser
 */

JSONParser::~JSONParser()
{
}

/****************************************************************************/
/*																			*/
/*	Internal Constants														*/
/*																			*/
/****************************************************************************/

/*	JSONParser::warn
 *
 *		Print warning
 */

void JSONParser::warn(const char *msg, ...)
{
	char buffer[512];
	
	if (!warnings) return;
	
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	errors.push_back(JSONError(lexer->getLine(),true,buffer));
}

/*	JSONParser::error
 *
 *		Print warning
 */

void JSONParser::error(const char *msg, ...)
{
	char buffer[512];
		
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);
	
	errors.push_back(JSONError(lexer->getLine(),false,buffer));
}

/****************************************************************************/
/*																			*/
/*	Internal Constants														*/
/*																			*/
/****************************************************************************/

/*	JSONParser::parse
 *
 *		Parse the input file, issuing warnings if the warning flag is set. If
 *	this returns false, there was a problem parsing the results
 */
 
bool JSONParser::parse(JSONLexer *l, bool w)
{
	lexer = l;
	warnings = w;
	
	errors.clear();
	
	/*
	 *	Read the next object. This recursively decides how to handle the
	 *	rest
	 */
	
	return parseValue();
}

/*	JSONParser::parseValue
 *
 *		Parse the value. This assumes we are at the start of a value to parse,
 *	and assumes the next token is either a start of object, start of array,
 *	true, false, null, a number or a string
 */

bool JSONParser::parseValue()
{
	int token = lexer->readToken();
	
	bool done = false;
	
	while (!done) {
		if (token == -1) {
			error("unexpected EOF");
			return false;
		} else if (token == '{') {
			parseObject();
		} else if (token == '[') {
			parseArray();
		} else if (token == TOKEN) {
			std::string t = lexer->token;
			
			if (t == "true") {
				boolean(true);
			} else if (t == "false") {
				boolean(false);
			} else if (t == "null") {
				null();
			} else {
				/*
				 *	Unexpected string token in stream. Error and abort
				 */
				 
				error("token %s illegal",t.c_str());
				return false;
			}
		} else if (token == STRING) {
			string(lexer->token);
		} else if (token == NUMBER) {
			std::string t = lexer->token;

			/*
			 *	Parse number
			 */
			 
			// If this has a '.' in it, assume float, otherwise integer
			if (t.find('.') == std::string::npos) {
				int64_t val = atoll(t.c_str());
				integer(val);
			} else {
				double val = atof(t.c_str());
				real(val);
			}
		} else {
			/*
			 *	Unexpected token in the stream. Warn and skip
			 */
			std::string t = lexer->token;
			warn("token %s unexpected",t.c_str());
			token = lexer->readToken();
			continue;
		}
	
		done = true;
	}
	return true;
}

/*	JSONParser::parseObject
 *
 *		Parse the object. This is called after the '{' token is seen
 */

bool JSONParser::parseObject()
{
	bool success = true;
	bool tailComma = false;
	
	startObject();
	
	for (;;) {
		/*
		 *	key: value, ...
		 */
		
		int token = lexer->readToken();
		if (token == '}') {
			if (tailComma) {
				warn("close after comma");
			}
			break;
		}
		if (token == ']') {
			warn("close array instead of close object");
			break;
		}
		
		if (token != STRING) {
			warn("expected object key as a string");
		}
		
		objectKey(lexer->token);
		
		/*
		 *	Check ':'
		 */
		
		token = lexer->readToken();
		if (token != ':') {
			warn("expected ':' separating key from value");
			lexer->pushToken();
		}
		
		/*
		 *	Read value
		 */
		
		success &= parseValue();
		
		/*
		 *	Read terminating ','
		 */
		
		token = lexer->readToken();
		if (token == -1) {
			success = false;
			error("unexpected EOF");
			break;
		}
		if (token == '}') {
			break;
		}
		if (token == ']') {
			warn("close array instead of close object");
			break;
		}
		if (token != ',') {
			warn("expected ',' separating key/value pairs in object");
			lexer->pushToken();
		}
		tailComma = true;
	}
	
	endObject();
	
	return success;
}

/*	JSONParser::parseArray
 *
 *		Parse the array object
 */

bool JSONParser::parseArray()
{
	bool success = true;
	bool tailComma = false;
	
	/*
	 *	This is a loop of 'value', 'value'...
	 */
	
	startArray();
	for (;;) {
		int token = lexer->readToken();
		
		if (token == ']') {
			if (tailComma) {
				warn("close after comma");
			}
			break;
		}
		if (token == '}') {
			warn("close object instead of close array");
			break;
		}

		lexer->pushToken();
		success &= parseValue();
		
		token = lexer->readToken();
		if (token == -1) {
			success = false;
			error("unexpected EOF");
			break;
		}
		if (token == ']') {
			break;
		}
		if (token == '}') {
			warn("close object instead of close array");
			break;
		}
		if (token != ',') {
			warn("comma expected between array values");
			lexer->pushToken();
		}
		tailComma = true;
	}
	
	endArray();
	return success;
}

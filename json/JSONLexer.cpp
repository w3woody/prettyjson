//
//  JSONLexer.cpp
//  JSONSyntaxChecker
//
//  Created by William Woody on 11/16/20.
//

#include "JSON.h"

/****************************************************************************/
/*																			*/
/*	JSON Lexer																*/
/*																			*/
/****************************************************************************/

/*	JSONLexer::JSONLexer
 *
 *		Lexer engine
 */

JSONLexer::JSONLexer(FILE *f)
{
	file = f;
	pos = 0;
	line = 1;
	pushBack = false;
}

JSONLexer::~JSONLexer()
{
}

/****************************************************************************/
/*																			*/
/*	JSON Lexer																*/
/*																			*/
/****************************************************************************/

/*	JSONLexer::readChar
 *
 *		Read the next character in the stream. Note this reads as 8-bit bytes,
 *	and does not validate unicode as UTF-8 characters.
 */

int JSONLexer::readChar()
{
	int ret;
	
	if (pos > 0) {
		ret = stack[--pos];
	} else {
		ret = fgetc(file);
	}
	
	if (ret == '\n') ++line;
	return ret;
}

void JSONLexer::pushChar(int ch)
{
	stack[pos++] = (uint8_t)ch;
	
	if (ch == '\n') --line;
}

/*	toHexValue
 *
 *		Convert character to hex
 */

static int toHexValue(char ch)
{
	if ((ch >= '0') && (ch <= '9')) return ch - '0';
	if ((ch >= 'a') && (ch <= 'f')) return 10 + ch - 'a';
	if ((ch >= 'A') && (ch <= 'F')) return 10 + ch - 'A';
	return 0;
}

/*	JSONLexer::readToken
 *
 *		Read the next token in the stream
 */

int JSONLexer::readToken()
{
	int c;
	
	/*
	 *	If we need to, return last token
	 */
	
	if (pushBack) {
		pushBack = false;
		return lastToken;
	}
	
	/*
	 *	Skip whitespace
	 */
	
	while (isspace(c = readChar())) ;
	if (c == -1) return -1;			/* At EOF */
	
	/*
	 *	Parse strings
	 */
	
	token.clear();
	if (c == '"') {
		while (((c = readChar()) != '"') && (c != -1)) {
			if (c == '\\') {
				/*
				 *	Escape processing.
				 */
				
				c = readChar();
				if (c == 'u') {
					/*
					 *	Hex escape. Note hex escape runs 0x0000 to 0xFFFF
					 */
					 
					uint16_t hex = 0;
					int ct = 0;
					while (ct++ < 4) {
						c = readChar();
						if (isxdigit(c)) {
							hex = (hex << 4) | toHexValue(c);
						} else {
							pushChar(c);
							break;
						}
					}
					
					/*
					 *	Convert the hex value to UTF-8.
					 */
					
					if (hex < 0x80) {
						token.push_back((char)hex);
					} else if (hex < 0x800) {
						token.push_back((char)(0xC0 | (hex >> 6)));
						token.push_back((char)(0x80 | (0x3F & hex)));
					} else {
						token.push_back((char)(0xE0 | (hex >> 12)));
						token.push_back((char)(0x80 | (0x3F & (hex >> 6))));
						token.push_back((char)(0x80 | (0x3F & hex)));
					}
				} else {
					token.push_back((char)c);
				}
			} else {
				token.push_back((char)c);
			}
		}
		
		return lastToken = STRING;
	}
	
	if (isalpha(c)) {
		/*
		 *	Parse token.
		 */
		
		while (isalnum(c) || (c == '_')) {
			token.push_back((char)c);
			c = readChar();
		}
		pushChar(c);
		
		return lastToken = TOKEN;
	}
	
	if (isdigit(c) || (c == '-')) {
		token.push_back((char)c);
		
		/*
		 *	Parse numeric value. (If we start with a minus sign, make sure the
		 *	next token is a digit. If not, then it's a minus sign.)
		 */
		
		if (c == '-') {
			c = readChar();
			if (!isdigit(c)) {
				pushChar(c);
				
				return lastToken = '-';
			} else {
				token.push_back((char)c);
			}
		}
		
		/*
		 *	At this point we've seen a single digit. Continue gathering digits
		 *	until we hit a decimal or something else.
		 */
		
		for (;;) {
			c = readChar();
			if (isdigit(c)) {
				token.push_back((char)c);
			} else {
				break;
			}
		}
		
		if (c == '.') {
			token.push_back((char)c);
			
			for (;;) {
				c = readChar();
				if (isdigit(c)) {
					token.push_back((char)c);
				} else {
					break;
				}
			}
		}
		
		if ((c == 'e') || (c == 'E')) {
			token.push_back((char)c);
			
			c = readChar();
			if ((c == '-') || (c == '+')) {
				token.push_back((char)c);
			} else {
				pushChar(c);
			}
			
			for (;;) {
				c = readChar();
				if (isdigit(c)) {
					token.push_back((char)c);
				} else {
					break;
				}
			}
		}
		
		pushChar(c);
		return lastToken = NUMBER;
	}
	
	/*
	 *	Return single character
	 */
	
	token.push_back((char)c);
	return lastToken = c;
}

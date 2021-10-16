//
//  JSON.h
//  JSONSyntaxChecker
//
//  Created by William Woody on 11/16/20.
//

#ifndef JSON_h
#define JSON_h

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <stdint.h>

/****************************************************************************/
/*																			*/
/*	JSON Objects															*/
/*																			*/
/****************************************************************************/

/*	JSON*/

/****************************************************************************/
/*																			*/
/*	Internal Constants														*/
/*																			*/
/****************************************************************************/

/*
 *	Internal tokens
 */
 
#define STRING		256
#define TOKEN		257
#define NUMBER		258

/****************************************************************************/
/*																			*/
/*	JSON Parser																*/
/*																			*/
/****************************************************************************/

/*	JSONLexer
 *
 *		JSON Lexer engine
 */

class JSONLexer
{
	public:
						JSONLexer(FILE *f);
						~JSONLexer();
	
		int				readToken();
		void			pushToken()
							{
								pushBack = true;
							}
						
		
		std::string		token;
		uint32_t		getLine()
							{
								return line;
							}

	private:
		FILE			*file;
		uint32_t		line;
		
		uint8_t			pos;
		uint8_t			stack[8];
		
		int				readChar();
		void			pushChar(int ch);
		
		bool			pushBack;
		int				lastToken;
};

/*	JSONError
 *
 *		Encapsulates a parser error
 */

class JSONError
{
	public:
						JSONError(long l, bool w, std::string s) : line(l), warning(w), str(s)
							{
							}
							
		bool			isWarning()
							{
								return warning;
							}
	
		long			getLine()
							{
								return line;
							}
		std::string		getError()
							{
								return str;
							}
							
	private:
		bool			warning;
		long			line;
		std::string		str;
};

/*	JSONParser
 *
 *		JSON Parser and syntax checker
 */
 
class JSONParser
{
	public:
						JSONParser();
		virtual			~JSONParser();
						
		bool			parse(JSONLexer *lexer, bool warnings);
		
		/*
		 *	SAX-like interface
		 */
		
		virtual void	null() = 0;
		virtual void	boolean(bool value) = 0;
		virtual void	integer(int64_t value) = 0;
		virtual void	real(double value) = 0;
		virtual void	string(std::string &value) = 0;
		
		virtual void	startArray() = 0;
		virtual void	endArray() = 0;
		
		virtual void	startObject() = 0;
		virtual void	endObject() = 0;
		virtual void	objectKey(std::string &value) = 0;

		/*
		 *	Found errors
		 */
		
		std::vector<JSONError> errors;
		
	protected:
		void			warn(const char *msg, ...);
		void			error(const char *msg, ...);

	private:
		bool			parseObject();
		bool			parseArray();
		bool			parseValue();
		
		JSONLexer		*lexer;
		bool			warnings;
};

/****************************************************************************/
/*																			*/
/*	DOM Parser																*/
/*																			*/
/****************************************************************************/

/*	JSONType
 *
 *		Type declaration
 */
 
enum JSONType {
	JSONTypeObject,
	JSONTypeArray,
	JSONTypeString,
	JSONTypeNumber,
	JSONTypeBoolean,
	JSONTypeNull
};

/*	JSONNode
 *
 *		Each potential value in our json object is represented by a node
 */

class JSONNode
{
	public:
						JSONNode();
		virtual			~JSONNode();
						
		virtual JSONType type() = 0;
};

class JSONObject: public JSONNode, public std::map<std::string, JSONNode *>
{
	public:
						JSONObject();
						~JSONObject();
	
		JSONType		type();
};

class JSONArray: public JSONNode, public std::vector<JSONNode *>
{
	public:
						JSONArray();
						~JSONArray();
	
		JSONType		type();
};

class JSONString: public JSONNode, public std::string
{
	public:
						JSONString();
						JSONString(std::string &val);
						~JSONString();
	
		JSONType		type();
};

class JSONNumber: public JSONNode
{
	public:
						JSONNumber(int64_t value);
						JSONNumber(double value);
						JSONNumber(bool value);
						~JSONNumber();
						
		JSONType		type();
		bool			isIntegerValue()
							{
								return isInteger;
							}
		
		int64_t			intValue();
		double			realValue();
		bool			boolValue();
	
	private:
		bool			isBoolean;
		bool			isInteger;
		union {
			bool		bvalue;
			int64_t		ivalue;
			double		rvalue;
		} u;
};

class JSONNull: public JSONNode
{
	public:
						JSONNull();
						~JSONNull();
	
		JSONType		type();
};

/*	JSONRecordParser
 *
 *		Parse into a JSON object
 */

class JSONRecordParser: public JSONParser
{
	public:
						JSONRecordParser();
		virtual			~JSONRecordParser();
		
		JSONNode		*parse(JSONLexer *lexer);
		
		/*
		 *	Interface
		 */
		
		void			null();
		void			boolean(bool value);
		void			integer(int64_t value);
		void			real(double value);
		void			string(std::string &value);
		
		void			startArray();
		void			endArray();
		
		void			startObject();
		void			endObject();
		void			objectKey(std::string &value);
							
	private:
		void			addValue(JSONNode *node);
		
		JSONNode		*root;
		std::string		key;
		std::vector<JSONNode *> stack;
};


#endif /* JSON_h */

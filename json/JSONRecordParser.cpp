//
//  JSONRecordParser.cpp
//  DecompileOnboarding
//
//  Created by William Woody on 9/10/21.
//

#include "JSON.h"

/****************************************************************************/
/*																			*/
/*	JSON Objects															*/
/*																			*/
/****************************************************************************/

/*	JSONNode */

JSONNode::JSONNode()
{
}

JSONNode::~JSONNode()
{
}

/* JSONObject */

JSONObject::JSONObject()
{
}

JSONObject::~JSONObject()
{
	// This runs the map and deletes the contents explicitly
	std::map<std::string,JSONNode *>::iterator iter;
	
	for (iter = begin(); iter != end(); ++iter) {
		delete iter->second;
	}
}

JSONType JSONObject::type()
{
	return JSONTypeObject;
}

/* JSONArray */

JSONArray::JSONArray()
{
}

JSONArray::~JSONArray()
{
	// This runs the array and deletes the contents explicitly
	size_t i,len = size();
	for (i = 0; i < len; ++i) {
		JSONNode *node = (*this)[i];
		delete node;
	}
}

JSONType JSONArray::type()
{
	return JSONTypeArray;
}

/* JSONString */

JSONString::JSONString()
{
}

JSONString::JSONString(std::string &val) : std::string(val)
{
}

JSONString::~JSONString()
{
}

JSONType JSONString::type()
{
	return JSONTypeString;
}

/* JSONNull */

JSONNull::JSONNull()
{
}

JSONNull::~JSONNull()
{
}

JSONType JSONNull::type()
{
	return JSONTypeNull;
}

/* JSONNumber
 *
 *		Note: we use the same object for booleans, because some of our interfaces
 *	allow 0/1 integers to interchange with boolean true/false. G'figure.
 */

JSONNumber::JSONNumber(int64_t val)
{
	u.ivalue = val;
	isBoolean = false;
	isInteger = true;
}

JSONNumber::JSONNumber(double val)
{
	u.rvalue = val;
	isBoolean = false;
	isInteger = false;
}

JSONNumber::JSONNumber(bool val)
{
	u.bvalue = val;
	isBoolean = true;
	isInteger = false;
}

JSONNumber::~JSONNumber()
{
}

JSONType JSONNumber::type()
{
	return isBoolean ? JSONTypeBoolean : JSONTypeNumber;
}

bool JSONNumber::boolValue()
{
	if (isBoolean) {
		return u.bvalue;
	} else if (isInteger) {
		return u.ivalue != 0;
	} else {
		return u.rvalue != 0;
	}
}

double JSONNumber::realValue()
{
	if (isBoolean) {
		return u.bvalue ? 1 : 0;
	} else if (isInteger) {
		return u.ivalue;
	} else {
		return u.rvalue;
	}
}

int64_t JSONNumber::intValue()
{
	if (isBoolean) {
		return u.bvalue ? 1 : 0;
	} else if (isInteger) {
		return u.ivalue;
	} else {
		return u.rvalue;
	}
}

/****************************************************************************/
/*																			*/
/*	JSON Parser																*/
/*																			*/
/****************************************************************************/

/*	JSONRecordParser::JSONRecordParser
 *
 *		Start up
 */

JSONRecordParser::JSONRecordParser()
{
}

JSONRecordParser::~JSONRecordParser()
{
	size_t i,len = stack.size();
	for (i = 0; i < len; ++i) {
		delete stack[i];
	}
}

/*	JSONRecordParser::parse
 *
 *		Parse the next item
 */

JSONNode *JSONRecordParser::parse(JSONLexer *lexer)
{
	/*
	 *	Wipe out the old stack.
	 */

	size_t i,len = stack.size();
	for (i = 0; i < len; ++i) {
		delete stack[i];
	}
	stack.clear();
	root = NULL;
	
	/*
	 *	Start up the parser
	 */
	
	bool err = JSONParser::parse(lexer,true);
	if (!err) return NULL;				// On error, give up.
	
	/*
	 *	We should have one object on the stack. If we don't, something went
	 *	haywire (like an unbalanced stack).
	 */
	
	if (stack.size() != 0) {
		error("Unbalanced close array and object markers make file invalid");
		return NULL;
	}
	
	return root;
}

/****************************************************************************/
/*																			*/
/*	JSON Parser																*/
/*																			*/
/****************************************************************************/

void JSONRecordParser::addValue(JSONNode *n)
{
	if (root == NULL) {
		root = n;
	} else if (stack.size() < 1) {
		error("INTERNAL ERROR: Multiple values without wrapped object");
	} else {
		JSONNode *node = stack.back();
		if (node->type() == JSONTypeObject) {
			JSONObject *obj = dynamic_cast<JSONObject *>(node);
			(*obj)[key] = n;
		} else if (node->type() == JSONTypeArray) {
			JSONArray *array = dynamic_cast<JSONArray *>(node);
			array->push_back(n);
		} else {
			error("INTERNAL ERROR: Wrapped object not on stack");
		}
	}
}

void JSONRecordParser::null()
{
	addValue(new JSONNull);
}

void JSONRecordParser::boolean(bool val)
{
	addValue(new JSONNumber(val));
}

void JSONRecordParser::integer(int64_t val)
{
	addValue(new JSONNumber(val));
}

void JSONRecordParser::real(double val)
{
	addValue(new JSONNumber(val));
}

void JSONRecordParser::string(std::string &val)
{
	addValue(new JSONString(val));
}

void JSONRecordParser::startArray()
{
	JSONArray *newArray = new JSONArray;
	
	addValue(newArray);				// Add empty array to the container
	stack.push_back(newArray);
}

void JSONRecordParser::endArray()
{
	stack.pop_back();
}

void JSONRecordParser::startObject()
{
	JSONObject *newObject = new JSONObject;
	
	addValue(newObject);
	stack.push_back(newObject);
}

void JSONRecordParser::endObject()
{
	stack.pop_back();
}

void JSONRecordParser::objectKey(std::string &value)
{
	key = value;
}


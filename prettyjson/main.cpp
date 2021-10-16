//
//  main.cpp
//  prettyjson
//
//  Created by William Woody on 10/15/21.
//

#include <iostream>
#include "JSON.h"

/****************************************************************************/
/*																			*/
/*	Print Contents															*/
/*																			*/
/****************************************************************************/

/*	JSONEscapeString
 *
 *		Determine if the string needs to be escaped to put into a JSON string
 */

static std::string JSONEscapeString(const std::string &str)
{
	const uint8_t *s = (const uint8_t *)str.c_str();
	const uint8_t *ptr;
	std::string ret;
	
	ptr = s;
	while (*ptr) {
		if (*ptr == '"') {
			ret.append("\\\"");
		} else if (*ptr == '\\') {
			ret.append("\\\\");
		} else if (*ptr == '\b') {
			ret.append("\\b");
		} else if (*ptr == '\f') {
			ret.append("\\f");
		} else if (*ptr == '\n') {
			ret.append("\\n");
		} else if (*ptr == '\r') {
			ret.append("\\r");
		} else if (*ptr == '\t') {
			ret.append("\\t");
		} else if (*ptr >= 0x80) {
			uint16_t val;
			
			if (*ptr <= 0xE0) {
				val = 0x1F & *ptr;
				val = (val << 6) | (0x3F & *++ptr);
			} else {
				val = 0x0F & *ptr;
				val = (val << 6) | (0x3F & *++ptr);
				val = (val << 6) | (0x3F & *++ptr);
			}
			
			char buffer[32];
			sprintf(buffer,"\\u%04X",val);
			ret.append(buffer);
		} else {
			ret.push_back(*ptr);
		}
		
		ptr++;
	}
	
	return ret;
}

static void JSONPrintString(const std::string &str)
{
	std::string f = JSONEscapeString(str);
	printf("\"%s\"",f.c_str());
}

static void JSONIndent(int depth)
{
	for (int i = 0; i <= depth; ++i) printf("  ");
}

static void JSONFormat(JSONNode *node, int depth = 0, bool sameLine = false)
{
	if (node->type() == JSONTypeObject) {
		JSONObject *obj = dynamic_cast<JSONObject *>(node);
		printf("{ ");
		bool first = true;
		std::map<std::string, JSONNode *>::iterator iter;
		for (iter = obj->begin(); iter != obj->end(); iter++) {
			if (first) {
				first = false;
				if (sameLine) {
					printf("\n");
					JSONIndent(depth);
				}
			} else {
				printf(", \n");
				JSONIndent(depth);
			}
			
			JSONPrintString(iter->first);
			printf(": ");
			JSONFormat(iter->second, depth+1, true);
		}
		printf("\n");
		JSONIndent(depth-1);
		printf("}");
		
	} else if (node->type() == JSONTypeArray) {
		JSONArray *array = dynamic_cast<JSONArray *>(node);
		printf("[ ");
		bool first = true;
		std::vector<JSONNode *>::iterator iter;
		for (iter = array->begin(); iter != array->end(); iter++) {
			if (first) {
				first = false;
				if (sameLine) {
					printf("\n");
					JSONIndent(depth);
				}
			} else {
				printf(", \n");
				JSONIndent(depth);
			}
			
			JSONFormat(*iter, depth+1, true);
		}
		printf("\n");
		JSONIndent(depth-1);
		printf("]");
	
	} else if (node->type() == JSONTypeString) {
		JSONString *s = dynamic_cast<JSONString *>(node);
		JSONPrintString(*s);
	} else if (node->type() == JSONTypeNumber) {
		char buffer[64];
		JSONNumber *n = dynamic_cast<JSONNumber *>(node);
		if (n->isIntegerValue()) {
			sprintf(buffer,"%lld",n->intValue());
		} else {
			sprintf(buffer,"%f",n->realValue());
		}
		printf("%s",buffer);
	} else if (node->type() == JSONTypeBoolean) {
		JSONNumber *n = dynamic_cast<JSONNumber *>(node);
		printf(n->boolValue() ? "true" : "false");
	} else {
		printf("null");
	}
}

/****************************************************************************/
/*																			*/
/*	Run parser																*/
/*																			*/
/****************************************************************************/

int main(int argc, const char * argv[])
{
	FILE *f;
	bool isStdin = true;
	
	if (argc > 1) {
		f = fopen(argv[1],"rb");
		if (f == NULL) {
			printf("Unable to open file\n");
			exit(1);
		}
		isStdin = false;
	} else {
		f = stdin;
	}
	JSONLexer lexer(f);
	JSONRecordParser parser;
	JSONNode *node = parser.parse(&lexer);
	
	/*
	 *	Dump the errors at the top
	 */
	
	std::vector<JSONError>::iterator iter;
	for (iter = parser.errors.begin(); iter != parser.errors.end(); ++iter) {
		printf("# line %ld: %s %s\n",iter->getLine(),iter->isWarning() ? "W" : "E",iter->getError().c_str());
	}
	
	/*
	 *	Print the formatted stuff
	 */
	 
	if (node != NULL) {
		JSONFormat(node);
		printf("\n");
	}
	
	if (!isStdin) {
		fclose(f);
	}

	return 0;
}

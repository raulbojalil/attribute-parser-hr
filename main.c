#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define PARSER_STATE_NONE 0
#define PARSER_STATE_READING_TAGNAME 1
#define PARSER_STATE_READING_ATTRNAME 2
#define PARSER_STATE_READING_ATTRVAL 3
#define PARSER_STATE_READING_CONTENT 4

#define MAX_CODE_LENGTH 1024
#define MAX_LINE_LENGTH 100
#define MAX_NAME_ATTR_LENGTH 50

typedef struct a
{
	char name[MAX_NAME_ATTR_LENGTH];
	char value[MAX_NAME_ATTR_LENGTH];
} attribute;

typedef struct t tag;

typedef struct t
{
	char name[MAX_NAME_ATTR_LENGTH];
	attribute* attrs[10];
    tag* children[30];
	int attrsCount;
	int childrenCount;
} tag;

void readChildren(char* code, tag* parent, int* pos)
{
	int parserState = PARSER_STATE_NONE;
	tag* currentTag = NULL;
	attribute* currentAttr = NULL;
	char textLength = 0;
	int quotes = 0;

	while (code[*pos] != '\0')
	{
		char currentChar = code[*pos];
		char nextChar = code[*pos + 1];

		switch (parserState)
		{
		case PARSER_STATE_NONE:
			if (currentChar == '<' && nextChar != '/') { //Verificar que no venga con un /
				currentTag = (tag*)malloc(sizeof(tag));
				currentTag->attrsCount = 0;
				currentTag->childrenCount = 0;
				parserState = PARSER_STATE_READING_TAGNAME;
			}
			else if (currentChar == '/')
				return;

			break;
		case PARSER_STATE_READING_TAGNAME:
			if (currentChar == ' ' || currentChar == '>') {

				currentTag->name[textLength] = '\0';
				textLength = 0;

				if (currentChar == '>') parserState = PARSER_STATE_READING_CONTENT;
				else
				{
					currentAttr = (attribute*)malloc(sizeof(attribute));
					parserState = PARSER_STATE_READING_ATTRNAME;
				}
				
			}
			else if(currentChar != '\n')
				currentTag->name[textLength++] = currentChar;
			break;
		case PARSER_STATE_READING_ATTRNAME:
			if (currentChar == '=')
			{
				parserState = PARSER_STATE_READING_ATTRVAL;
				currentAttr->name[textLength] = '\0';
				textLength = 0;
			}
			else if(currentChar == '>')
				parserState = PARSER_STATE_READING_CONTENT;
			else if (currentChar != ' ' && currentChar != '\n')
				currentAttr->name[textLength++] = currentChar;

			break;
		case PARSER_STATE_READING_ATTRVAL:

			if (currentChar != '\"') {
				if(quotes == 1 && currentChar != '\n')
					currentAttr->value[textLength++] = currentChar;
			}
			else
				quotes++;

			if (quotes == 2)
			{
				quotes = 0;
				currentAttr->value[textLength] = '\0';
                textLength = 0;
				currentTag->attrs[currentTag->attrsCount++] = currentAttr;

				if (currentChar == '>') parserState = PARSER_STATE_READING_CONTENT;
				else {
					currentAttr = (attribute*)malloc(sizeof(attribute));
					parserState = PARSER_STATE_READING_ATTRNAME;
				}

			}

			break;
		case PARSER_STATE_READING_CONTENT:

			if (currentChar == '<' && nextChar != '/') {
				readChildren(code, currentTag, pos);
			}
			else if (currentChar == '>')
			{
				parent->children[parent->childrenCount++] = currentTag;
				parserState = PARSER_STATE_NONE;
			}
			break;
		}

		*pos = *pos + 1;
	}

}
void getAttr(tag* t, char* query, int pos)
{
	char attrName[MAX_NAME_ATTR_LENGTH] = "";
    int len = strlen(query) - pos;
	memcpy(attrName, &query[pos], len);
    attrName[len - 1] = attrName[len - 1] == '\n' ? '\0' : attrName[len - 1];
    
	for (int i = 0; i < t->attrsCount; i++)
	{
		if (strcmp(t->attrs[i]->name, attrName) == 0)
		{
			printf("%s\n", t->attrs[i]->value);
			return;
		}
	}

	printf("Not Found!\n");
}

void getTag(tag* t, char* query, int* pos)
{
	char tagName[MAX_NAME_ATTR_LENGTH];
	int i = 0;
	char isAttribute = 0;
	
	while (query[*pos] != '\0')
	{
		if (query[*pos] == '.' || query[*pos] == '~')
		{
			tagName[i] = '\0';
			isAttribute = query[*pos] == '~' ? 1 : 0;
			*pos = *pos + 1;
			break;
		}
		else
			tagName[i++] = query[*pos];

	    *pos = *pos + 1;
	}

	for (i = 0; i < t->childrenCount; i++)
	{
		if (strcmp(t->children[i]->name, tagName) == 0)
		{
			if (isAttribute) getAttr(t->children[i], query, *pos);
			else getTag(t->children[i], query, pos);
			return;
		}
	}

	printf("Not Found!\n");
}

void printTags(tag* t, int indentation)
{
	for (int i = 0; i < indentation; i++)
	{
		printf(" ");
	}

	printf("%s:", t->name);

	for (int i = 0; i < t->attrsCount; i++)
	{
		printf("[%s=%s]", t->attrs[i]->name, t->attrs[i]->value);
	}
	printf("\n");

	for (int i = 0; i < t->childrenCount; i++)
	{
		printTags(t->children[i], indentation + 4);
	}

}

int main() {

    int lineCount = 0;
    int queryCount = 0;
    
    char code[MAX_CODE_LENGTH] = "";
    
    scanf("%d %d", &lineCount, &queryCount);
    int offset = 0;
    for(int i=0; i < lineCount+1; i++)
    {
        char line[MAX_LINE_LENGTH];
        fgets(line, MAX_LINE_LENGTH, stdin);
        if(i > 0)
        {
            memcpy(&code[offset], line, strlen(line)-1);
            offset += strlen(line)-1;
        }
        
    }
    
    //printf("%s\n", code);   
    
    tag root;
	root.attrsCount = 0;
	root.childrenCount = 0;
	int pos = 0;

	readChildren(code, &root, &pos);
   
    //printTags(&root, 0);
    
    for(int i=0; i < queryCount; i++)
    {
        char line[MAX_LINE_LENGTH];
        fgets(line, MAX_LINE_LENGTH, stdin);
        pos = 0;
        getTag(&root, line, &pos);
    }
    
    return 0;
}

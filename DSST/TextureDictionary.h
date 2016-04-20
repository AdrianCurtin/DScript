
#ifndef __GL_H__
typedef unsigned int GLuint;
#endif

#ifndef TEX_H
#define TEX_H

#include <stdio.h>

class TextureDictionaryItem
{
public:
	TextureDictionaryItem(void);
	TextureDictionaryItem(int iValue,char* fname);
	~TextureDictionaryItem(void);

	int key;	// index value: 0,1,2...
	int value;  // id value: unique number for each texture...
	char fname[800];

	TextureDictionaryItem *next;

};


class TextureDictionary
{
public:
	TextureDictionaryItem* cListRoot;

	int GetValue(int iKey);
	int GetKey(int iValue);
	void Clear();
	int GetLength();
	TextureDictionaryItem* GetLast();
	int Add(int iValue, char* fname);

	TextureDictionary(void);
	~TextureDictionary(void);
};

#endif


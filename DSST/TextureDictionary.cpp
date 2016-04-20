#include <stdio.h>
#include <stdlib.h>

#include "TextureDictionary.h"



TextureDictionary::TextureDictionary(void)
{
	cListRoot = new TextureDictionaryItem();
}

TextureDictionary::~TextureDictionary(void)
{
}

int TextureDictionary::GetValue(int iKey)
{
	TextureDictionaryItem* cur = cListRoot;
	while(cur->next)
	{
		if(cur->key == iKey)
			return cur->value;
		cur=cur->next;
	}
	return -1;
}

int TextureDictionary::GetKey(int iValue)
{
	TextureDictionaryItem* cur = cListRoot;
	while(cur->next)
	{
		if(cur->value == iValue)
			return cur->key;
		cur=cur->next;
	}
	if(cur->value == iValue)
		return cur->key;
	return 0;
}


void TextureDictionary::Clear(void)
{
	//clear the list...
	TextureDictionaryItem *temp,*cur;
	cur = cListRoot->next;

	if(cur!=NULL)
	{	
		GLuint tex;
		tex=cur->key;
		//glDeleteTextures(1, &tex);
		while(cur->next)
		{
			temp = cur;

			GLuint tex;
			tex=cur->key;
			//glDeleteTextures(1, &tex); //frees texture from GL

			cur=cur->next;
			//delete temp;
			free(temp);
		}
	}
	cListRoot->next=NULL;
}

int TextureDictionary::GetLength()
{
	TextureDictionaryItem *ret=cListRoot;
	int count=0;
	while(ret->next!=NULL)
	{
		ret = ret->next;
		count++;
	}
	return count;
}

TextureDictionaryItem* TextureDictionary::GetLast()
{
	TextureDictionaryItem *ret=cListRoot;
	while(ret->next!=NULL)
	{
		ret = ret->next;
	}
	return ret;
}

int TextureDictionary::Add(int iValue,char* fname)
{
	TextureDictionaryItem *cur = GetLast();
	int i=GetLength()+1;
	cur->next = new TextureDictionaryItem(iValue,fname);
	return i;
}

TextureDictionaryItem::TextureDictionaryItem(void)
{
	next=0;
	key=-100;
	value=-100;
}

TextureDictionaryItem::TextureDictionaryItem(int iValue, char* fname)
{
	next=0;
	sprintf(this->fname,"%s",fname);
	value=iValue;
}

TextureDictionaryItem::~TextureDictionaryItem(void)
{
}

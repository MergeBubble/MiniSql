#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#include "base.h"
#include "const.h"
#include <fstream>
#include <time.h>
#include <string.h>
using namespace std;

class insertPos{
public:
	int bufferNUM;
	int position;
};
class buffer{
public:
	bool isWritten;
	bool isValid;
	string filename;
	int blockOffset;	//block offset in file, indicate position in file 	
	int LRUvalue;		
	int recent_time; //show the recent visit time
	char values[BLOCKSIZE + 1];
	buffer(){
		initialize();
	}
	void initialize(){
		isWritten = 0;
		isValid = 0;
		filename = "NULL";
		blockOffset = 0;
		memset(values,EMPTY,BLOCKSIZE); 
		values[BLOCKSIZE] = '\0';
	}
	string getvalues(int startpos, int endpos){
		string tmpt = "";
		if (startpos >= 0 && startpos <= endpos && endpos <= BLOCKSIZE)
			for (int i = startpos; i < endpos; i++)
				tmpt += values[i];
		return tmpt;
	}
	char getvalues(int pos){
		if (pos >= 0 && pos <= BLOCKSIZE)
			return values[pos];
		return '\0';
	}
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	void flashBack(int bufferNum);//½«blockÁ¢¿ÌÐ´ÈëÎÄ¼þ
	
	int getbufferNum(string filename, int blockOffset);//»ñÈ¡Ö¸¶¨blockÔÚÄÚ´æÖÐµÄ±àºÅ
	
	void readBlock(string filename, int blockOffset, int bufferNum); //½«ÎÄ¼þ¿é¶ÁÈ¡µ½blockÖÐ
	void writeBlock(int bufferNum);//已经删除
	void useBlock(int bufferNum); //已经删除
	
	int getEmptyBuffer();//Ñ°ÕÒÄÚ´æÖÐ¿ÕµÄblock
	int getEmptyBufferExcept(string filename);
	int getIfIsInBuffer(string filename, int blockOffset);//ÕÒµ½Ö¸¶¨blockÔÚÄÚ´æÖÐµÄ±àºÅ
	insertPos getInsertPosition(Table& tableinfor);//·µ»Ø²åÈëÊý¾ÝµÄ¿ÉÐÐÎ»ÖÃ
	
	int addBlockInFile(Table& tableinfor);//ÎÄ¼þºó²åÈëÐÂµÄblock£¬·µ»ØÐÂµÄblockÔÚÄÚ´æÖÐµÄ±àºÅ
	//int addBlockInFile(Index& indexinfor);

	void scanIn(Table tableinfo);//°ÑÕû¸ö±íÎÄ¼þÈ«²¿¶¼¶ÁÈëÄÚ´æÖÐ£¨É÷ÓÃ£¬Èç¹ûÎÄ¼þÌ«´ó£¬ËùÓÐµÄblockÕ¼Âú¶¼¶Á²»Íê£¬»á±ÀÀ£¡£¡£¡£¡££©
	void setInvalid(string filename);
	
	int GiveMeABlock(string filename, int blockOffset); //Íâ²¿º¯Êýµ÷ÓÃ½Ó¿Ú

	friend class RecordManager;
	friend class CataManager;
//private:
	buffer bufferBlock[MAXBLOCKNUM];
};


#endif

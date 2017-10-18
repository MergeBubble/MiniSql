//
// Created by 37187 on 2017/6/7.
//

#ifndef CPLUS_BPTREE_H
#define CPLUS_BPTREE_H

#include "iostream"
#include "fstream"
#include <cstdlib>
#include <cstring>
#include "BufferManager.h"

#define BSize 4096

using namespace std;
extern BufferManager bf;


class BpTree {
public:
	int rootblock;
	int NodeNumber;
private:
	int type;
	int MaxNumber;
	string name;
	const int keylength[3]={4,4,10};

public:
	BpTree(string bname);
	virtual ~BpTree(){};

	void Initialize(Data* key, int addr);
	int Search(Data* key);
	void Insert(Data* key, int addr);
	void split_leaf(char* node,char* node1);
	void split_internal(char* node,char* node1, int son1,int son2);
	void insert_internal(char* node,int son1,int son2);
	int find_leaf(Data* key);
	void Initialize(char* node);
	void Delete(Data* key);
	int* Range(Data* key1,Data* key2);
};




#endif //CPLUS_BPTREE_H

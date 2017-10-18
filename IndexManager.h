//
// Created by 37187 on 2017/6/7.
//

#ifndef CPLUS_INDEXMANAGER_H
#define CPLUS_INDEXMANAGER_H

#include<string>
#include<iostream>
#include<fstream>
#include"bptree.h"
#include"base.h"

class IndexManager{
public:
	IndexManager(){};
	virtual ~IndexManager(){};

	void Establish(string file);
	void Insert(string file, Data* key, int addr);
	int Search(string file, Data* key);
	void Delete(string file);  //函数重载
	int *RSearch(string file, Data* key1, Data* key2);
	void Delete(string file, Data* key);
};

#endif //CPLUS_INDEXMANAGER_H

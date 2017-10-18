//
// Created by 37187 on 2017/6/7.
//

#include "IndexManager.h"


void IndexManager::Establish(string name)
{
    BpTree bp(name);
}
void IndexManager::Insert(string name, Data* key, int addr)
{
    BpTree bp(name);
    if(bp.NodeNumber==0){
        bp.Initialize(key,addr); //第一处改动，flag保持一致，且char型改成1.
    }
    else{
        bp.Insert(key,addr);
    }
}

int IndexManager::Search(string name, Data* key){
    BpTree bp(name);
    if(bp.NodeNumber==0){
//        cout<<"Can't find the key! There is no index!";
        return -1;
    }
    else{
        return bp.Search(key);
    }
}
void IndexManager::Delete(string name)
{
    remove(name.c_str());
}
int IndexManager::*RSearch(string name, Data* key1, Data* key2)
{
    int *a=new int[5];
}

void IndexManager::Delete(string file, Data *key) {

}
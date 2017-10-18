#ifndef _RECORDMANAGER_H
#define	_RECORDMANAGER_H
#include "base.h"
#include "const.h"
#include "BufferManager.h"
#include "Catalog.h"
class RecordManager
{
public:
    RecordManager(BufferManager *bf):buf_ptr(bf){}
    ~RecordManager();
    bool isSatisfied(Table& tableinfor, tuper& row, vector<int> mask, vector<where> w);
    Table Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w);
    Table SelectWithIndex(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w);
    Table Select(Table& tableIn, vector<int>attrSelect);
    int FindWithIndex(Table& tableIn, tuper& row, int mask);
    void Insert(Table& tableIn, tuper& singleTuper);
    char* MakeAstring(Table& tableIn, tuper& singleTuper);
    int Delete(Table& tableIn, vector<int>mask, vector<where> w);
    bool DropTable(Table& tableIn);
    bool CreateTable(Table& tableIn);
    Table SelectProject(Table& tableIn, vector<int>attrSelect);

private:
    RecordManager(){}
    BufferManager *buf_ptr;
};

#endif

//
// Created by 37187 on 2017/6/7.
//

#include <iostream>
#include "Interpreter.h"
#include "API.h"

using namespace std;

BufferManager bf;
extern int Number;
int main() {
    int re = 1;
    InterManager itp;
    cout << "MiniSQL By Zhb & Hdc!" << endl ;
    while(re){
        try{
            cout << ">>>";
            itp.GetQuery();
            re = itp.EXEC();
        }
        catch(TableException te){
            cout << te.what() << endl;
        }
        catch(QueryException qe){
            cout << qe.what() << endl;
        }
    }
    return 0;
}

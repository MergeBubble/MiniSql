#include "RecordManager.h"
#include "InterPreter.h"
#include "IndexManager.h"
#include <cmath>
RecordManager::~RecordManager()
{
}
//------ 执行select语句  是有where语句的select语句 ------//
Table RecordManager:: Select(Table& tableIn ,vector<int>attrSelect ,vector<int>mask , vector<where>& w  )
{
    //如果条件项数 为零  那么直接返回
    cout<<mask.size()<<endl ;
    cout<<mask[0]<<endl ;
    if (mask.size() == 0)
    {
        return Select(tableIn , attrSelect) ;
    }
        //select with index
    else if (tableIn.attr.unique[ mask[0] ] == 1)   //如果 where 中的一个条件参数是主键
    {
        cout<<"index serch"<<endl ;
        return SelectWithIndex(tableIn,attrSelect,mask,w);
    }
    //select with index
    string stringRow ;
    string filename = tableIn.getname() + ".table";   //得到表的名字
    string indexfilename ;      //index 文件名
    int length = tableIn.dataSize() +1 ;    // 长度记录 表中一条数据的大小+1
    const int recordNum = BLOCKSIZE /length ;  //一个BLOCK 存放有recordNum个记录
    buf_ptr->scanIn( tableIn);
    for (int i = 0  ; i < tableIn.blockNum ; i ++  )
    {
        int bufferNum = buf_ptr->getIfIsInBuffer(filename , i );
        for ( int j = 0 ; j < recordNum ; j ++)
        {
            int  position =j * length ;
            stringRow = buf_ptr->bufferBlock[ bufferNum ].getvalues(position , position + length);
            if (stringRow.c_str()[0] == EMPTY) continue ;
            int c_pos= 1 ;
            tuper * temp_tuper = new tuper;
            for( int attrNUm = 0 ; attrNUm < tableIn.getattribute().num ; attrNUm ++ )
            {
                if (tableIn.getattribute().flag[ attrNUm ]  == -1 )            //当前属性是一个 int 型的变量
                {
                    int temp ;
                    memcpy (&temp ,&(stringRow.c_str()[c_pos]) , sizeof(int));
                    c_pos += sizeof(int );
                    temp_tuper->addData(new Datai( temp ));
                }
                else if (tableIn.getattribute().flag[attrNUm] == 0 )        //=float
                {
                    float temp;
                    memcpy(&temp , &(stringRow.c_str()[c_pos] ) ,sizeof(float) );
                    c_pos += sizeof(float);
                    temp_tuper->addData( new Dataf( temp ));
                }
                else if ( tableIn.getattribute().flag[attrNUm] > 0 )
                {
                    char temp[ MAXSTRINGLEN] ;
                    int strLen = tableIn.getattribute().flag[ attrNUm ] +1 ;
                    memcpy ( &temp , &(stringRow.c_str()[ c_pos]) , strLen);
                    c_pos += strLen ;
                    temp_tuper->addData (new Datac(string(temp )));
                }
            }

            if ( isSatisfied( tableIn , * temp_tuper ,mask ,w ) )
                tableIn.addData(temp_tuper );
            else delete temp_tuper ;
        }
    }
    return SelectProject(tableIn ,attrSelect );
}


Table RecordManager :: Select (Table& tableIn , vector<int> attrSelect  )
{
    string stringRow ;
    string filename = tableIn.getname() +".table";
    tuper* temp_tuper = new tuper ;
    buf_ptr -> scanIn (tableIn) ;
    int length = tableIn.dataSize ( ) +1 ;
    const int recordNum = BLOCKSIZE /length ;
    for(int i = 0 ; i < tableIn.blockNum ; i ++ )
    {
        int bufferNum = buf_ptr-> getIfIsInBuffer (filename , i );
        for( int  j ; j < recordNum ; j++ )
        {
            int position  = j * length ;
            stringRow =buf_ptr->bufferBlock[ bufferNum ].getvalues (position , position + length );
            if ( stringRow.c_str()[0] == EMPTY) continue ;
            int flag_pos = 1 ;
            tuper * temp_tuper = new tuper ;
            for ( int attrNum = 0 ; attrNum < tableIn.getattribute().num ; attrNum ++ )
            {
                if (tableIn.getattribute().flag[ attrNum ] == -1 )   //int
                {
                    int temp ;
                    memcpy(&temp , &(stringRow.c_str()[ flag_pos]) , sizeof(int )) ;
                    flag_pos += sizeof(int );
                    temp_tuper->addData(new Datai( temp ));
                }
                else if (tableIn.getattribute().flag[ attrNum ] == 0 ) //float
                {
                    float temp ;
                    memcpy (&temp , & (stringRow.c_str()[ flag_pos] ) , sizeof(float ));
                    flag_pos += sizeof(float );
                    temp_tuper ->addData(new Dataf(temp));
                }
                else if (tableIn.getattribute().flag[ attrNum ] > 0 ) //char
                {
                    char temp[MAXSTRINGLEN];
                    int LEN = tableIn.getattribute().flag[ attrNum ] +1 ;
                    memcpy(&temp ,& (stringRow.c_str()[ flag_pos] ) , LEN);
                    flag_pos += LEN ;
                    temp_tuper ->addData(new Datac (string(temp)));
                }
            }
            tableIn.addData(temp_tuper );
        }
    }
    return SelectProject(tableIn ,attrSelect);

}


Table RecordManager::SelectWithIndex(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w)
{
    IndexManager indexMA;
    cout<<"end"<<endl ;
    string filename = tableIn.getname() + ".table";
    string stringRow ;
    tuper* temp_tuper = new tuper ;
    int length = tableIn.dataSize() + 1 ;
    const int recordNum = BLOCKSIZE / length ;
    //case : a lot of query condintion
    int* re ;
    int addr = -1;
    Data* minData ;
    Data* maxData ;


    int w_size = mask.size() ;
    for(int i = 0 ; i < w_size ; i++ )
    {
        Data* ptrData ;
        switch( (w[i].d)->flag )
        {
            case -1: ptrData = (Datai*)w[i].d  ; break ;
            case 0: ptrData =  (Dataf*)w[i].d ;break ;
            default: ptrData = (Datac*)w[i].d ;break ;
        }
        if(w[i].flag == eq )
            addr =indexMA.Search(tableIn.getname()+ to_string ( mask[0] )+".index"  ,  ptrData) ;    //table 的第i个属性 作为查询条件
        else if(w[i].flag == l ||w[i].flag == leq)
            maxData = ptrData ;
        else if (w[i].flag == g ||w[i].flag == geq)
            minData = ptrData ;
    }
    // ** //
    /*int x = (w[0].d)->flag ;
    Data* ptrData ;
    switch(x)
    {
        case -1: ptrData = (Datai*)w[0].d  ; break ;
        case 0: ptrData =  (Dataf*)w[0].d ;break ;
        default: ptrData = (Datac*)w[0].d ;break ;
    }
    int addr =indexMA.Find(tableIn.getname()+ to_string ( mask[0] )+".index"  ,  ptrData) ;    //table 的第i个属性 作为查询条件
    */
    cout<<"addr:"<<addr<<endl;
    if( addr >= 0){
        int offest = addr /recordNum ;
        int position = addr % recordNum *length ;
        int bufferNum = buf_ptr->GiveMeABlock(filename ,offest);
        int c_pos= 1 ;
        stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position,position+length);
        for (int attr_index = 0; attr_index < tableIn.getattribute().num;attr_index++)
        {
            if (tableIn.getattribute().flag[attr_index] == -1){                   // 如果当前这个  attr_Index 号 属性  是 int 型
                int value;
                memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));             // 那么 就用一个 int 型 变量 把 这个东西 弄出来
                c_pos += sizeof(int);                                                    //c_pos后移
                temp_tuper->addData(new Datai(value));                                   // 将value 新增到 数据元祖中
            }
            else if (tableIn.getattribute().flag[attr_index]==0){                     // 如果为 float
                float value;
                memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
                c_pos += sizeof(float);
                temp_tuper->addData(new Dataf(value));
            }
            else{
                char value[MAXSTRINGLEN];
                int strLen = tableIn.getattribute().flag[attr_index]+1;
                memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
                c_pos += strLen;
                temp_tuper->addData(new Datac(string(value)));
            }
        }
        tableIn.addData(temp_tuper);
        return SelectProject(tableIn , attrSelect);
    }

}



//利用 index 进行查询 的 子函数
int RecordManager::FindWithIndex(Table& tableIn, tuper& row, int mask)
{
    IndexManager indexMA;
    for (int i = 0; i < tableIn.index.num; i++) {            //遍历一个表的所有 index
        if (tableIn.index.location[i] == mask) {                // 如果 i号主键的属性位置  ==   mask  为传入的 属性号
            Data* ptrData;
            ptrData = row[mask];
            int pos = indexMA.Search(tableIn.getname() + to_string(mask) + ".index", ptrData);       //
            return pos;
        }
    }
    return -1;
}



// ------判断单条元祖是否满足插入的条件  ---------//
bool RecordManager::isSatisfied(Table& tableinfor, tuper& row, vector<int> mask, vector<where> w)
{
    bool res = true;
    for (int i = 0; i < mask.size();i++){
        if (w[i].d == NULL){ //
            continue;
        }
        else if ( row[mask[i]]->flag == -1 ) { //int
            switch (w[i].flag) {
                case eq: if ( !(((Datai*)row[mask[i]])->x == ((Datai*)w[i].d)->x) ) return false;break;
                case leq: if (!(((Datai*)row[mask[i]])->x <= ((Datai*)w[i].d)->x)) return false; break;
                case l: if (!(((Datai*)row[mask[i]])->x < ((Datai*)w[i].d)->x)) return false; break;
                case geq: if (!(((Datai*)row[mask[i]])->x >= ((Datai*)w[i].d)->x)) return false; break;
                case g: if (!(((Datai*)row[mask[i]])->x >((Datai*)w[i].d)->x)) return false; break;
                case neq: if (!(((Datai*)row[mask[i]])->x != ((Datai*)w[i].d)->x)) return false; break;
                default: ;
            }
        }
        else if (row[mask[i]]->flag == 0) { //Float
            switch (w[i].flag) {
                case eq: if (!(abs(((Dataf*)row[mask[i]])->x - ((Dataf*)w[i].d)->x)<MIN_Theta)) return false; break;
                case leq: if (!(((Dataf*)row[mask[i]])->x <= ((Dataf*)w[i].d)->x)) return false; break;
                case l: if (!(((Dataf*)row[mask[i]])->x < ((Dataf*)w[i].d)->x)) return false; break;
                case geq: if (!(((Dataf*)row[mask[i]])->x >= ((Dataf*)w[i].d)->x)) return false; break;
                case g: if (!(((Dataf*)row[mask[i]])->x >((Dataf*)w[i].d)->x)) return false; break;
                case neq: if (!(((Dataf*)row[mask[i]])->x != ((Dataf*)w[i].d)->x)) return false; break;
                default: ;
            }
        }
        else if (row[mask[i]]->flag > 0){ //string
            switch (w[i].flag) {
                case eq: if (!(((Datac*)row[mask[i]])->x == ((Datac*)w[i].d)->x)) return false; break;
                case leq: if (!(((Datac*)row[mask[i]])->x <= ((Datac*)w[i].d)->x)) return false; break;
                case l: if (!(((Datac*)row[mask[i]])->x < ((Datac*)w[i].d)->x)) return false; break;
                case geq: if (!(((Datac*)row[mask[i]])->x >= ((Datac*)w[i].d)->x)) return false; break;
                case g: if (!(((Datac*)row[mask[i]])->x >((Datac*)w[i].d)->x)) return false; break;
                case neq: if (!(((Datac*)row[mask[i]])->x != ((Datac*)w[i].d)->x)) return false; break;
                default: ;
            }
        }
        else { //just for debug
            cout << "Error in RecordManager in function is satisified!" << endl;
            system("pause");
        }
    }
    return res;
}

void RecordManager :: Insert( Table& tableIn , tuper& singleTuper)
{
    for(int i = 0 ; i < tableIn.attr.num ; i++ )
    {
        if(tableIn.attr.unique[ i ] == 1   )  //刚刚创建 会有一个 bug
        {

            if( FindWithIndex(tableIn , singleTuper , i ) >= 0)
            {
                throw QueryException("Unique value Redunancy occurs !");
                return ;
            }
        }
    }
    for (int i = 0; i < tableIn.attr.num; i++) {
        if (tableIn.attr.unique[i]) {
            vector<where> w;
            vector<int> mask;
            where *uni_w = new where;
            uni_w->flag = eq;
            switch (singleTuper[i]->flag) {
                case -1:uni_w->d = new Datai(((Datai*)singleTuper[i])->x); break;
                case 0:uni_w->d = new Dataf(((Dataf*)singleTuper[i])->x); break;
                default:uni_w->d = new Datac(((Datac*)singleTuper[i])->x); break;
            }
            w.push_back(*uni_w);
            mask.push_back(i);
            Table temp_table = Select(tableIn, mask, mask, w);


            if (temp_table.T.size() > 0) {
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
            }
            delete uni_w->d;
            delete uni_w;
        }
    }
    tableIn.addData(&singleTuper);

    char * temp_tuper ;
    temp_tuper = MakeAstring(tableIn , singleTuper);
    insertPos Position =  buf_ptr ->getInsertPosition(tableIn );
    buf_ptr->bufferBlock[ Position.bufferNUM ].values[Position.position] = NOTEMPTY ;
    memcpy (& (buf_ptr->bufferBlock[ Position.bufferNUM ] .values[Position.position +1] ) ,temp_tuper ,     tableIn.dataSize() );//插入记录
    int length = tableIn.dataSize() +1 ;
    IndexManager IM ;
    int recordNum = BLOCKSIZE / length ;
    for (int i = 0 ; i < tableIn.index.num  ; i++ )
    {
        int insert_addr = buf_ptr -> bufferBlock[Position.bufferNUM].blockOffset * recordNum + Position.position /length ;  //解决index
        for (int j = 0 ; j < tableIn.index.num ;j ++ )
        {
            IM.Insert(tableIn.getname() + to_string(tableIn.index.location[j])+".index", singleTuper[ tableIn.index.location[i]] ,insert_addr );
        }
    }
    buf_ptr->writeBlock(Position.bufferNUM);
    delete[] temp_tuper ;
}
//将表单数据简单整合成 字符串
char* RecordManager:: MakeAstring (Table& tableIn , tuper& singleTuper )
{
    char* result ;
    int c_pos =0 ;
    result = new char[ (tableIn.dataSize() + 1) * sizeof(char )] ;
    for (int i = 0 ; i < tableIn.attr.num ; i++ )
    {
        if(tableIn.getattribute().flag[i] == -1 )    // int 型
        {
            int temp ;
            temp = ((Datai*)singleTuper[i] )-> x ;
            memcpy ( result + c_pos , & temp , sizeof(int ));
            c_pos += sizeof(int );
        }
        else if (tableIn.getattribute().flag[i] == 0)
        {
            float temp ;
            temp = ( (Dataf*)singleTuper[i])-> x ;
            memcpy(result + c_pos ,&temp ,sizeof(float)) ;
            c_pos += sizeof(float );
        }
        else if (tableIn.getattribute().flag[i] > 0 )
        {
            string temp(((Datac*)singleTuper[i]) ->x );
            int Len = tableIn.getattribute().flag[i] + 1 ;
            memcpy(result + c_pos , &temp , Len) ;
            c_pos += Len;
        }
    }
    result [tableIn.dataSize()  ] = '\0' ;
    return result ;
}
//删除table
int RecordManager::Delete(Table& tableIn , vector<int>mask , vector<where > w )
{
    int count = 0 ;
    string stringRow ;
    string filename = tableIn.getname() + ".table";
    buf_ptr->scanIn(tableIn) ;
    int length = tableIn.dataSize() + 1  ;
    const int recordNum = BLOCKSIZE/length ;
    for(int i = 0 ; i < tableIn.blockNum ; i++ )
    {
        int bufferNum = buf_ptr->getIfIsInBuffer(filename , i ) ;
        for (int j = 0 ; j < recordNum ; j++)
        {
            tuper *temp_tuper = new tuper ;
            int c_pos = 1 ;
            int position = length * j ;
            stringRow = buf_ptr->bufferBlock[ bufferNum ] .getvalues(position , position + length );
            for( int attrNUm = 0 ; attrNUm < tableIn.attr.num ; attrNUm ++ )
            {
                if (tableIn.attr.flag[ attrNUm ] == -1 ) //int
                {
                    int temp ;
                    memcpy(&temp , & (stringRow.c_str()[c_pos]) , sizeof(int ) ) ;
                    c_pos += sizeof(int );
                    temp_tuper->addData(new Datai(temp));

                }
                else if (tableIn.attr.flag [attrNUm ] == 0 ) // float
                {
                    float temp ;
                    memcpy(& temp , & (stringRow.c_str()[c_pos ] ) , sizeof(float ) );
                    c_pos += sizeof(float);
                    temp_tuper->addData(new Dataf(temp ));
                }
                else if (tableIn.attr.flag[attrNUm ] > 0)
                {
                    char temp[MAXSTRINGLEN] ;
                    int Len = tableIn.attr.flag[ attrNUm ] +1 ;
                    memcpy(& temp , &(stringRow.c_str()[c_pos]) , Len );
                    c_pos += Len ;
                }
            }
            if( isSatisfied(tableIn , *temp_tuper , mask , w ) )
            {
                buf_ptr -> bufferBlock[ bufferNum].values[position] = EMPTY ;
                buf_ptr -> writeBlock(bufferNum) ;
                count++ ;
            }
        }
        return count ;
    }
}

//droptable 操作
bool RecordManager::DropTable(Table& tableIn)
{
    string filename = tableIn.getname()+".table" ;
    if ( remove(filename.c_str()) != 0 )
    {
        throw TableException("Can't drop the table !") ;
    }
    else buf_ptr -> setInvalid(filename) ;
    return true ;
}
// 未涉及重写
//create table 操作
bool RecordManager::CreateTable(Table& tableIn)
{
    string filename = tableIn.getname() + ".table";
    fstream fout(filename.c_str(), ios::out);
    fout.close();
    tableIn.blockNum = 1;
    CataManager Ca;
    Ca.changeblock(tableIn.getname(), 1);
    return true;
}


//对选择出来的 元祖进行投影 使得结果只有给定属性
Table RecordManager::SelectProject(Table& tableIn, vector<int>attrSelect) {
    Attribute attrOut;
    tuper *ptrTuper = NULL;
    attrOut.num = attrSelect.size();      // 一共有 attrout.Num个属性
    for (int i = 0; i < attrSelect.size(); i++) {                  //遍历 这些选择出来的 所有属性
        attrOut.flag[i] = tableIn.getattribute().flag[attrSelect[i]];       //  int -1 ——float 0 —— char other
        attrOut.name[i] = tableIn.getattribute().name[attrSelect[i]];       //   名字
        attrOut.unique[i] = tableIn.getattribute().unique[attrSelect[i]];    // 是否唯一
    }
    Table tableOut(tableIn.getname(), attrOut, tableIn.blockNum);    //初始化
    int k;
    for (int i = 0; i < tableIn.T.size(); i++) {                //
        ptrTuper = new tuper;
        for (int j = 0; j < attrSelect.size(); j++) {          //一共有   attrSelect.size() 种属性被选择
            k = attrSelect[j];                              //
            Data *resadd = NULL;
            if (tableIn.T[i]->operator[](k)->flag == -1) {                 //寻址
                resadd = new Datai((*((Datai *) tableIn.T[i]->operator[](k))).x);
            } else if (tableIn.T[i]->operator[](k)->flag == 0) {
                resadd = new Dataf((*((Dataf *) tableIn.T[i]->operator[](k))).x);
            } else if (tableIn.T[i]->operator[](k)->flag > 0) {
                resadd = new Datac((*((Datac *) tableIn.T[i]->operator[](k))).x);
            }

            ptrTuper->addData(resadd);//bug

        }
        tableOut.addData(ptrTuper);
    }

    return tableOut;}
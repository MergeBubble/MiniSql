#include "BufferManager.h"
#include "Catalog.h"

#include <stdio.h>

BufferManager::BufferManager()
{
    for (int i = 0; i< MAXBLOCKNUM; i++)
        bufferBlock[i].initialize();// 对于所有的buffer 子类都进行  初始化
}

BufferManager::~BufferManager()
{
    //std::cout << "~buffer" << endl;
    for (int i = 0; i < MAXBLOCKNUM; i++)
        flashBack(i);                // 对于所有的 buffer 都立即写回文件
}
//将 block 立即写回文件
void BufferManager::flashBack(int bufferNum)   //对于第 bufferNum 个 内存块 操作
{
    if(!bufferBlock[bufferNum].isWritten) return;   // 如果这个内存块  已经被写过 那么 return；

    string filename = bufferBlock[bufferNum].filename;
    //fstream fout;
    FILE *fp;                               //文件指针
    if ((fp = fopen(filename.c_str(),"r+b"))==NULL	){
        cout << "Open file error!" << endl; // #Todo
        return;
    }
    fseek(fp, BLOCKSIZE*bufferBlock[bufferNum].blockOffset, SEEK_SET);  // 将文件指针指向偏移量计算之后
    fwrite(bufferBlock[bufferNum].values, BLOCKSIZE, 1, fp);    // 将buffer 中的 values 写回到文件当中去
    bufferBlock[bufferNum].initialize();    //写回之后 对这个 buffer 进行初始化操作
    fclose(fp);               //把文件关闭
    /*fout.open(filename.c_str(), ios::in | ios::out);
    fout.seekp(BLOCKSIZE*bufferBlock[bufferNum].blockOffset, fout.beg);
    fout.write(bufferBlock[bufferNum].values, BLOCKSIZE);
    bufferBlock[bufferNum].initialize();
    fout.close();*/
}

//寻找指定文件是否在内存中 如果在，则找到指定block 	在内存中的编号
int BufferManager::getIfIsInBuffer(string filename, int blockOffset){
    for (int bufferNum = 0; bufferNum < MAXBLOCKNUM; bufferNum++)
        if (bufferBlock[bufferNum].filename == filename && bufferBlock[bufferNum].blockOffset == blockOffset)
            return bufferNum;
    return -1;
}
//获取指定block 在内存中的编号
int BufferManager::getbufferNum(string filename, int blockOffset)
{
    int bufferNum = getIfIsInBuffer(filename, blockOffset);
    if (bufferNum == -1){   // 如果这个文件并没有找到  没有在内存当中 ，那么需要从文件中读取出来
        bufferNum = getEmptyBufferExcept(filename);    //  如果内存没用完 那么顺序遍历  找到第一个没有被使用的块   如果内存用完 则找到最长时间没有被访问的块
        readBlock(filename, blockOffset, bufferNum);   // 读取这个块
    }
    return bufferNum;
}
//寻找内存中空的 buffer 并且不能替换掉给定的文件
int BufferManager::getEmptyBufferExcept(string filename){
    int bufferNum = -1; //for invalid buffer block
    int least_recent_block = 0;
    for (int i = 0; i < MAXBLOCKNUM; i++)
    {
        if (!bufferBlock[i].isValid)   //找到第一个 未被使用的  Block
        {
            bufferBlock[i].initialize();    //初始化
            bufferBlock[i].isValid = 1;     // 标记 已被使用
            return i;                    //返回块的 值
        }
            //如果所有的内存块都在被使用
        else if (bufferBlock[least_recent_block].recent_time > bufferBlock[i].recent_time && bufferBlock[i].filename != filename)
        {
            least_recent_block = i;   // 找到 recent_time 最小的 那个块   代表最长时间没有被访问
        }
    }
    flashBack(least_recent_block);    // 找到最长时间没被访问的  块  把块中的东西 写回文件
    bufferBlock[least_recent_block].isValid = 1;     //然后把 这个块  占用情况赋值 为 1  代表被占用
    return least_recent_block;        //把这个块  的  块值  返回
}
//将文件读取到 buffer 中
void BufferManager::readBlock(string filename, int blockOffset, int bufferNum)
{
    bufferBlock[bufferNum].isValid = 1;          // 把这个  块 标记为 已读
    bufferBlock[bufferNum].isWritten = 0;           // 把这个 块标记为  未被 写回
    bufferBlock[bufferNum].filename = filename;           // 把这个块的 文件名 标记为要写入的 文件名
    bufferBlock[bufferNum].blockOffset = blockOffset;     // 把这个块的  偏移量 赋值为  偏移量
    bufferBlock[bufferNum].recent_time = clock();             // 把这个 recent_time 用 clock（） 记录下来

    FILE *fp;             //文件指针
    if ((fp = fopen(filename.c_str(),"rb"))==NULL){    //打开文件失败 则抛出异常
        cout << "Open file error" << endl;
        return;
    }
    fseek(fp, BLOCKSIZE*blockOffset, SEEK_SET);       //将文件指针  指到  块在文件中的偏移量 * 块的大小
    fread(bufferBlock[bufferNum].values, BLOCKSIZE, 1, fp);    //将fp 指针指向的 文件中的 值 读取出一个 块大小的  值写入  bufferNum下面的 values
    fclose(fp);   // 关闭文档

    /*fstream  fin;
    fin.open(filename.c_str(), ios::in | ios::binary);
    fin.seekp(BLOCKSIZE*blockOffset, ios::beg);
    fin.read(bufferBlock[bufferNum].values, BLOCKSIZE);
    fin.close();*/
}

// 外部接口  block被改写时调用 做标记
void BufferManager::
writeBlock(int bufferNum)
{
    bufferBlock[bufferNum].isWritten = 1 ;     //这个 buffer  被改写时 isWritten  标记
    useBlock(bufferNum) ;
}

//外部接口   读 block时调用 为该块做标记
void BufferManager::useBlock(int bufferNum)
{
    bufferBlock[bufferNum].recent_time = clock();   // 将这个bufferNUm 中 的 recent_time 赋值  代表使用过这个变量
}

//寻找内存中空的 buffer
int BufferManager::getEmptyBuffer()
{
    int bufferNum = -1; //for invalid buffer block
    int least_recent_block = 0;
    for (int i = 0; i < MAXBLOCKNUM;i++){
        if (!bufferBlock[i].isValid){
            //顺序遍历 找到第一个 没有被使用的 buffer 块   初始化  标记为被占用  将 块号返回
            bufferBlock[i].initialize();
            bufferBlock[i].isValid = 1;
            return i;
        }
            //若都被占用 则将最长时间没被占用的块 初始化 弄出来
        else if (bufferBlock[least_recent_block].recent_time > bufferBlock[i].recent_time){
            least_recent_block = i;
        }
    }
    flashBack(least_recent_block);
    bufferBlock[least_recent_block].isValid = 1;
    return least_recent_block;
}


//外部接口 返回插入数据的可行位置
insertPos BufferManager::getInsertPosition(Table& tableinfor){
    insertPos iPos;   //    包含buffernum 和 position 的 一个类
    if (tableinfor.blockNum == 0){ //new file and no block exist
        iPos.bufferNUM = addBlockInFile(tableinfor);   // 将tableinfor 拆入一个新的block  并且  接受返回的 block 在内存中的编号
        iPos.position = 0;
        return iPos;
    }
    string filename = tableinfor.getname() + ".table";
    int length = tableinfor.dataSize() + 1;
    int blockOffset = tableinfor.blockNum - 1;
    int bufferNum = getIfIsInBuffer(filename, blockOffset);
    if (bufferNum == -1){
        bufferNum = getEmptyBuffer();
        readBlock(filename, blockOffset, bufferNum);
    }
    int recordNum = BLOCKSIZE / length;
    for (int offset = 0; offset < recordNum;offset++){
        int position = offset * length;
        char isEmpty = bufferBlock[bufferNum].values[position];
        if (isEmpty == EMPTY){//find an empty space
            iPos.bufferNUM = bufferNum;
            iPos.position = position;
            return iPos;
        }
    }
    iPos.bufferNUM = addBlockInFile(tableinfor);
    iPos.position = 0;
    return iPos;
}
//文件后插入新的 block 返回新的block 在内存中的编号
int BufferManager::addBlockInFile(Table& tableinfor){
    int bufferNum = getEmptyBuffer();               //找到一个 新的 buffer
    bufferBlock[bufferNum].initialize();            // 初始化 其实可以不用
    bufferBlock[bufferNum].isValid = 1;             // 被占用 标记 为1
    bufferBlock[bufferNum].isWritten = 1;           //被改写 标记为 1
    bufferBlock[bufferNum].filename = tableinfor.getname() + ".table";    // 找到他的name  加上 ".table"
    bufferBlock[bufferNum].blockOffset = tableinfor.blockNum++;   // 新增一个 block 那么 这个块 在这个文件中的 偏移量  等于原来偏移量 +1
    bufferBlock[bufferNum].recent_time = clock();         //最近访问时间更新
    CataManager ca;
    ca.changeblock(tableinfor.getname(), tableinfor.blockNum);
    return bufferNum;
}

//把整个表文件读入内存当中
void BufferManager::scanIn(Table tableinfo){
    string filename = tableinfo.getname() + ".table";
    fstream  fin(filename.c_str(), ios::in);    //文件流
    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++)
    {
        if (getIfIsInBuffer(filename, blockOffset) == -1)   //内存中找不到 这个文件偏移量的东西
        {
            int bufferNum = getEmptyBufferExcept(filename);     // 那么开辟一个 block
            readBlock(filename, blockOffset, bufferNum);      //读取这个 偏移量的block
        }
    }
    fin.close();                                  //关闭
}
//将整个文件涉及的所有块 标记为无效 再删除表 和index 中使用
void BufferManager::setInvalid(string filename){
    for (int i = 0; i < MAXBLOCKNUM;i++){   //顺序扫描所有文件 将文件涉及的所有块 标记为无效
        if (bufferBlock[i].filename == filename){
            bufferBlock[i].isValid = 0;     // 标记未被使用
            bufferBlock[i].isWritten = 0;    // 标记未被 写入
        }
    }
}
// 外部接口    输入文件名和 偏移量  返回相应的内存编号
int BufferManager::GiveMeABlock(string filename, int blockOffset)
{
    int bufferNum = getIfIsInBuffer(filename, blockOffset);  //首先检测其是否  在 内存当中  有的话返回值
    if (bufferNum == -1) {                             // 如果不在
        bufferNum =getEmptyBuffer();                         //新增一个 buffer
        readBlock(filename, blockOffset, bufferNum);            //将 文件中的 指定偏移量的  文件信息 读入到 bufferNum buffer中去
    }                        											//那么 buffer 中就存下了   文件名 和 便宜量的信息
    useBlock(bufferNum);                                          // 使用这个 buffer
    return bufferNum;
}

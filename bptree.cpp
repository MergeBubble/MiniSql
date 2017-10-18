//
// Created by 37187 on 2017/6/7.
//

#include <fibersapi.h>
#include "BpTree.h"
//BufferManager bf;
BpTree::BpTree(string bname):name(bname){
	fstream file;
	file.open(name,ios::in);
	if(!file){
		file.open(name, ios::out);
		char *currentBlock = new char[BSize];
		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);

		*(int*)(currentBlock)=5;
		*(int*)(currentBlock+4)=-1;
		*(int*)(currentBlock+8)=0;
		*(int*)(currentBlock+12)=1;//root的block号
		memcpy(bf.bufferBlock[buffernum].values, currentBlock, BSize);
		bf.writeBlock(0);
		bf.flashBack(0);
		file.close();
	}
	else{
		char *currentBlock = new char[BSize];
		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BSize);
		type = *(int*)(currentBlock);
		MaxNumber=*(int*)(currentBlock+4);
		NodeNumber=*(int*)(currentBlock+8);
		rootblock=*(int*)(currentBlock+12);
		long size=file.tellg();
		MaxNumber=size/BSize-1;
		file.close();
	}


}

void BpTree::Initialize(Data* key, int addr){
	//定义第一个块，记录信息

	char *block=new char[BSize];
	if(key->flag<1)
		type=*(int*)(block)=key->flag+1;
	else type=*(int*)(block)=2;
	*(int*)(block+4)=MaxNumber;
	*(int*)(block+8)=0;
	*(int*)(block+12)=1;

// memcpy(block+12,name,name.length());
	int bn=bf.GiveMeABlock(name,0);
	memcpy(bf.bufferBlock[bn].values, block, BSize);
	bf.writeBlock(bn);
	int l=keylength[type]+16;
	//定义首个叶节点
	block=new char[BSize];
	Initialize(block);
	*(int*)(block)=1;
	*(int*)(block+4)=(BSize-16)/l-1;
	*(int*)(block+8)=1;
	*(int*)(block+12)=-1;   //根父节点为-1
	*(int*)(block+16+keylength[type]+4)=1;  //首个位置指向第一个值
	*(int*)(block+16+16+keylength[type]+keylength[type])=1;  //自己的编号
	*(int*)(block+16+16+keylength[type]+keylength[type]+4)=-1;//兄弟的编号
	*(int*)(block+16+16+keylength[type]+keylength[type]+8)=addr;//内存位置or比自己小的block位置
	*(int*)(block+16+16+keylength[type]+keylength[type]+12)=-1;//比自己大的block位置
	switch (type){
		case 0:
			*(int*)(block+16+l)=((Datai*)key)->x;
			break;
		case 1:
			*(int*)(block+16+l)=((Dataf*)key)->x;
			break;
		case 2:
			memcpy(block+16+l,((Datac*)key)->x.c_str(),((Datac*)key)->x.length()+1);
			break;
	}

	bn=bf.GiveMeABlock(name,1);
	memcpy(bf.bufferBlock[bn].values,block,BSize);
	bf.writeBlock(bn);
	NodeNumber++;
	bn=bf.GiveMeABlock(name,0);
	*(int*)(bf.bufferBlock[bn].values+8)=NodeNumber;
	bf.writeBlock(bn);
	delete[] block;
}
int BpTree::Search(Data* key){
	char *node=new char[BSize];
	int bn=bf.GiveMeABlock(name,0);
	memcpy(node,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);
	int root=*(int*)(node+12);

	node=new char[BSize];
	bn=bf.GiveMeABlock(name,root);
	memcpy(node,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);

	int l=keylength[type]+16;
	int nodetype=*(int*)(node+8);
	int brother=*(int*)(node+16+keylength[type]+4);
	while(nodetype==0){
		int a=0;
		int next;
		int i;
		while (brother!=-1) {
			if(type==0){
				int ikey=*(int*)(node+16+brother*l);
				if(((Datai*)key)->x<ikey){
					break;
				}
				a=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
			else if (type==1){
				float fkey=*(int*)(node+16+brother*l);
				if(((Dataf*)key)->x<fkey){
					break;
				}
				a=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
			else {
				string ckey((char*)(node+16+brother*l));
				if(((Datac*)key)->x.compare(ckey)<0) {
					break;
				}
				a=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
		}
		if(brother==-1){
			next=*(int*)(node+16+a*l+keylength[type]+12);
		}
		else{
			next=*(int*)(node+16+brother*l+keylength[type]+8);
		}

		bn=bf.GiveMeABlock(name,next);
		memcpy(node,bf.bufferBlock[bn].values,BSize);
		bf.useBlock(bn);
		nodetype=*(int*)(node+8);
		a=0;
		brother=*(int*)(node+16+keylength[type]+4);
	}
	int num=*(int*)(node);
	brother=*(int*)(node+16+keylength[type]+4);
	int addr;
	for(int i=0;i<num;i++){
		if(type==0){
			int ikey=*(int*)(node+16+brother*l);
			addr=*(int*)(node+16+brother*l+keylength[type]+8);
			if(((Datai*)key)->x==ikey){
				delete[] node;
				return addr;
			}
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
		else if (type==1){
			float fkey=*(int*)(node+16+brother*l);
			addr=*(int*)(node+16+brother*l+keylength[type]+8);
			if(((Dataf*)key)->x==fkey){
				delete[] node;
				return addr;
			}
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
		else {
			string ckey((char*)(node+16+brother*l));
			addr=*(int*)(node+16+brother*l+keylength[type]+8);
			if(((Datac*)key)->x.compare(ckey)==0) {
				delete[] node;
				return addr;
			}
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
	}
	delete[] node;
	return -1;
}
void BpTree::Insert(Data* key, int addr){
	int leaf=find_leaf(key);
	char *node=new char[BSize];
	int bn=bf.GiveMeABlock(name,leaf);
	int l=keylength[type]+16;
	memcpy(node,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);
	int num=*(int*)(node);
	int maxnum=*(int*)(node+4);

	if(num==maxnum){
		char *node1=new char[BSize];
		split_leaf(node,node1);
		memcpy(bf.bufferBlock[bn].values,node,BSize);
		bf.writeBlock(bn);

		//寻找父亲，开始递归
		bn=bf.GiveMeABlock(name,*(int*)(node+12));
		char *parent=new char[BSize];
		memcpy(parent,bf.bufferBlock[bn].values,BSize);
		bf.useBlock(bn);

		//完成整个树的更新
		if(*(int*)(parent)<*(int*)(parent+4)){
			//在父亲上添一个元素
			insert_internal(parent,leaf,NodeNumber);
		}
		else{
			char *parent2=new char[BSize];
			split_internal(parent, parent2,leaf,NodeNumber);
			delete[] parent2;
		}

		delete[] parent;

		leaf=find_leaf(key);
		bn=bf.GiveMeABlock(name,leaf);
		memcpy(node,bf.bufferBlock[bn].values,BSize);
		bf.useBlock(bn);
	}

	int brother=*(int*)(node+16+keylength[type]+4);
	int temp=0;
	*(int*)(node)=*(int*)(node)+1;
	while(brother!=-1){
		if(type==0){
			int ikey=*(int*)(node+16+brother*l);
			if(((Datai*)key)->x<ikey){
				break;
			}
			temp=brother;
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
		else if (type==1){
			float fkey=*(int*)(node+16+brother*l);
			if(((Dataf*)key)->x<fkey){
				break;
			}
			temp=brother;
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
		else {
			string ckey((char*)(node+16+brother*l));
			if(((Datac*)key)->x.compare(ckey)<0) {
				break;
			}
			temp=brother;
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
	}
	for(int i=1;i<=maxnum;i++){
		if(*(int*)(node+16+i*l+keylength[type]+4)==0){
			*(int*)(node+16+i*l+keylength[type])=i;
			*(int*)(node+16+i*l+keylength[type]+4)=brother;
			*(int*)(node+16+i*l+keylength[type]+8)=addr;
			*(int*)(node+16+i*l+keylength[type]+12)=-1;
			if(type==0){
				*(int*)(node+16+i*l)=((Datai*)key)->x;
			}
			else if(type==1){
				*(int*)(node+16+i*l)=((Dataf*)key)->x;
			}
			else{
				memcpy(node+16+i*l,((Datac*)key)->x.c_str(),((Datac*)key)->x.length()+1);
			}
			*(int*)(node+16+temp*l+keylength[type]+4)=i;
			break;
		}
	}
	memcpy(bf.bufferBlock[bn].values,node,BSize);
	bf.writeBlock(bn);
	bf.flashBack(bn);


}
int BpTree::find_leaf(Data *key) {
	char *node=new char[BSize];
	int bn=bf.GiveMeABlock(name,0);
	memcpy(node,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);
	int root=*(int*)(node+12);

	node=new char[BSize];
	bn=bf.GiveMeABlock(name,root);
	memcpy(node,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);

	int next=1;
	int l=keylength[type]+16;
	int nodetype=*(int*)(node+8);
	int brother=*(int*)(node+16+keylength[type]+4);
	while(nodetype==0){
		int temp=0;
		while(brother!=-1) {
			if(type==0){
				int ikey=*(int*)(node+16+brother*l);
				if(((Datai*)key)->x<ikey){
					break;
				}
				temp=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
			else if (type==1){
				float fkey=*(int*)(node+16+brother*l);
				if(((Dataf*)key)->x<fkey){
					break;
				}
				temp=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
			else {
				string ckey((char*)(node+16+brother*l));
				if(((Datac*)key)->x.compare(ckey)<0) {
					break;
				}
				temp=brother;
				brother=*(int*)(node+16+brother*l+keylength[type]+4);
			}
		}
		if(brother==-1){
			next=*(int*)(node+16+temp*l+keylength[type]+12);
		}
		else{
			next=*(int*)(node+16+brother*l+keylength[type]+8);
		}

		bn=bf.GiveMeABlock(name,next);
		memcpy(node,bf.bufferBlock[bn].values,BSize);
		bf.useBlock(bn);
		nodetype=*(int*)(node+8);
		temp=0;
		brother=*(int*)(node+16+keylength[type]+4);
	}
	return next;
}
void BpTree::Initialize(char *node) {
	int l=(keylength[type]+16);
	*(int*)(node)=0;
	*(int*)(node+4)=(BSize-16)/l-1;
	int maxnum=(BSize-16)/(keylength[type]+16)-1;
	*(int*)(node+8)=1;  //结点类型设置为叶结点
	*(int*)(node+12)=-1;//结点设置为根结点
	*(int*)(node+16+keylength[type]+4)=-1;
	for(int i=1;i<=maxnum;i++){
		*(int*)(node+16+i*l+keylength[type]+4)=0;
	}
}
//注意没有写进block
void BpTree::split_leaf(char *node,char *node1) {
	int temp;
	int maxnum=*(int*)(node+4);
	int l=keylength[type]+16;
	//结点初始化
	Initialize(node1);
	*(int*)(node1)=maxnum-maxnum/2;
	*(int*)(node1+12)=*(int*)(node+12);

	*(int*)(node)=maxnum/2;

	//分裂结点
	int brother=*(int*)(node+16+keylength[type]+4);
	for(int i=1;i<maxnum/2;i++){
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
	}

	*(int*)(node1+16+keylength[type]+4)=*(int*)(node+16+brother*l+keylength[type]+4);
	temp=brother;
	brother=*(int*)(node+16+brother*l+keylength[type]+4);
	*(int*)(node+16+temp*l+keylength[type]+4)=-1;

	while(brother!=-1){
		*(int*)(node1+16+brother*l+keylength[type]+4)=*(int*)(node+16+brother*l+keylength[type]+4);
		*(int*)(node1+16+brother*l+keylength[type])=*(int*)(node+16+brother*l+keylength[type]);
		*(int*)(node1+16+brother*l+keylength[type]+8)=*(int*)(node+16+brother*l+keylength[type]+8);
		*(int*)(node1+16+brother*l+keylength[type]+12)=*(int*)(node+16+brother*l+keylength[type]+12);
		if(type==0 || type==1){
			*(int*)(node1+16+brother*l)=*(int*)(node+16+brother*l);
		}
		else{
			memcpy(node1+16+brother*l,node+16+brother*l,20);
		}
		temp=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
		*(int*)(node+16+temp*l+keylength[type]+4)=0;

	}
	//如果分裂的点是根
	if(*(int*)(node+12)==-1){
		char *root=new char[BSize];
		Initialize(root);
		*(int*)(root+8)=0;
		int bn=bf.GiveMeABlock(name,0);
		*(int*)(bf.bufferBlock[bn].values+8)=*(int*)(bf.bufferBlock[bn].values+8)+1;
		NodeNumber=*(int*)(bf.bufferBlock[bn].values+8);
		bf.writeBlock(bn);

		bn=bf.GiveMeABlock(name,NodeNumber);
		memcpy(bf.bufferBlock[bn].values,root,BSize);
		bf.writeBlock(bn);

		*(int*)(node+12)=NodeNumber;
		*(int*)(node1+12)=NodeNumber;

		bn=bf.GiveMeABlock(name,0);
		*(int*)(bf.bufferBlock[bn].values+12)=NodeNumber;
		bf.writeBlock(bn);
	}

	int bn=bf.GiveMeABlock(name,0);
	*(int*)(bf.bufferBlock[bn].values+8)=*(int*)(bf.bufferBlock[bn].values+8)+1;
	NodeNumber=*(int*)(bf.bufferBlock[bn].values+8);
	bf.writeBlock(bn);

	bn=bf.GiveMeABlock(name,NodeNumber);
	memcpy(bf.bufferBlock[bn].values,node1,BSize);
	bf.writeBlock(bn);


}

void BpTree::split_internal(char *node, char *node1, int son1, int son2) {
	int flag;//检验结点在哪里重复，左边0右边1中间2
	int t;
	int temp;
	int maxnum=*(int*)(node+4);
	int l=keylength[type]+16;
	//结点初始化
	Initialize(node1);
	*(int*)(node1)=maxnum-maxnum/2;
	*(int*)(node1+8)=*(int*)(node+8);
	*(int*)(node1+12)=*(int*)(node+12);

	*(int*)(node)=maxnum/2;

	//添加新元素，并存储最右边的元素ikey
	int ikey,tikey;
	float fkey,tfkey;
	char ckey[20],tckey[20];
	int bn=bf.GiveMeABlock(name,son2);
	char *son=new char[BSize];
	memcpy(son,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);
	int temp1=*(int*)(bf.bufferBlock[bn].values+16+keylength[type]+4);
	int nodetype=*(int*)(bf.bufferBlock[bn].values+8);

	int brother=*(int*)(node+16+keylength[type]+4);
	while(brother!=-1){
		if(*(int*)(node+16+brother*l+keylength[type]+8)==son1){
			break;
		}
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
	}
	if(brother!=-1){
		if(type==0){
			ikey=*(int*)(node+16+brother*l);
			*(int*)(node+16+brother*l)=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else if(type==1){
			fkey=*(int*)(node+16+brother*l);
			*(int*)(node+16+brother*l)=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else{
			memcpy(ckey,node+16+brother*l,20);
			memcpy(node+16+brother*l,bf.bufferBlock[bn].values+16+temp1*l,20);
		}
		brother=*(int*)(node+16+brother*l+keylength[type]+4);

		while (brother!=-1){
			if(type==0){
				tikey=*(int*)(node+16+brother*l);
				*(int*)(node+16+brother*l)=ikey;
				ikey=tikey;
			}
			else if(type==1){
				tfkey=*(int*)(node+16+brother*l);
				*(int*)(node+16+brother*l)=fkey;
				fkey=tfkey;
			}
			else{
				memcpy(tckey,node+16+brother*l,20);
				memcpy(node+16+brother*l,ckey,20);
				memcpy(ckey,tckey,20);
			}
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}

	}
	else{
		if(type==0){
			ikey=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else if(type==1){
			fkey=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else{
			memcpy(ckey,bf.bufferBlock[bn].values+16+temp1*l,20);
		}
	}

	//分裂结点
	brother=*(int*)(node+16+keylength[type]+4);
	for(int i=1;i<maxnum/2;i++){
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
	}

	*(int*)(node1+16+keylength[type]+4)=*(int*)(node+16+brother*l+keylength[type]+4);
	temp=brother;
	brother=*(int*)(node+16+brother*l+keylength[type]+4);
	*(int*)(node+16+temp*l+keylength[type]+4)=-1;

	while(brother!=-1){
		*(int*)(node1+16+brother*l+keylength[type]+4)=*(int*)(node+16+brother*l+keylength[type]+4);
		*(int*)(node1+16+brother*l+keylength[type])=*(int*)(node+16+brother*l+keylength[type]);
		*(int*)(node1+16+brother*l+keylength[type]+8)=*(int*)(node+16+brother*l+keylength[type]+8);
		*(int*)(node1+16+brother*l+keylength[type]+12)=*(int*)(node+16+brother*l+keylength[type]+12);
		if(type==0 || type==1){
			*(int*)(node1+16+brother*l)=*(int*)(node+16+brother*l);
		}
		else{
			memcpy(node1+16+brother*l,node+16+brother*l,20);
		}
		temp=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
		*(int*)(node+16+temp*l+keylength[type]+4)=0;

	}
	int mostright=temp;

	//检测左边结点是否指向儿子
	int a=0;
	brother=*(int*)(node+16+keylength[type]+4);
	while(brother!=-1){
		if(*(int*)(node+16+brother*l+keylength[type]+8)==son1){
			break;
		}
		a=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
	}
	//如果有就把儿子结点往左顺移
	if(brother!=-1){
		flag=0;
		t=*(int*)(node+16+brother*l+keylength[type]+12);
		*(int*)(node+16+brother*l+keylength[type]+12)=son2;
		temp=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
		while (brother!=-1){
			int tt=*(int*)(node+16+brother*l+keylength[type]+12);
			*(int*)(node+16+brother*l+keylength[type]+12)=t;
			t=tt;
			*(int*)(node+16+brother*l+keylength[type]+8)=*(int*)(node+16+temp*l+keylength[type]+12);
			temp=brother;
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
	}
	else if(*(int*)(node+16+a*l+keylength[type]+12)==son1){
		flag=2;
		brother=*(int*)(node1+16+keylength[type]+4);
		*(int*)(node1+16+brother*l+keylength[type]+8)=son2;
	}
	else{
		//执行右边的相同操作：有就向右移
		flag=1;
		brother=*(int*)(node1+16+keylength[type]+4);
		temp=*(int*)(node1+16+brother*l+keylength[type]+4);
		while(brother!=-1){
			if(*(int*)(node1+16+brother*l+keylength[type]+12)==son1){
				break;
			}
			*(int*)(node1+16+brother*l+keylength[type]+8)=*(int*)(node1+16+brother*l+keylength[type]+12);
			*(int*)(node1+16+brother*l+keylength[type]+12)=*(int*)(node1+16+temp*l+keylength[type]+12);

			brother=*(int*)(node1+16+brother*l+keylength[type]+4);
			temp=*(int*)(node1+16+brother*l+keylength[type]+4);
		}
		*(int*)(node1+16+brother*l+keylength[type]+8)=son1;
		*(int*)(node1+16+brother*l+keylength[type]+12)=son2;
		if(temp!=-1){
			*(int*)(node1+16+temp*l+keylength[type]+8)=son2;
		}

	}
	//添加最右边的元素
	for(int i=1;i<=maxnum;i++){
		if(*(int*)(node1+16+i*l+keylength[type]+4)==0){
			*(int*)(node1+16+i*l+keylength[type]+4)=-1;
			*(int*)(node1+16+mostright*l+keylength[type]+4)=i;
			if(type==0){
				*(int*)(node1+16+i*l)=ikey;
			}
			else if(type==1){
				*(int*)(node1+16+i*l)=fkey;
			}
			else{
				memcpy(node1+16+i*l,ckey,20);
			}
			break;
		}
	}
	//如果分裂的点是根
	if(*(int*)(node+12)==-1){
		char *root=new char[BSize];
		Initialize(root);
		*(int*)(root+8)=0;
		int bt=bf.GiveMeABlock(name,0);
		*(int*)(bf.bufferBlock[bt].values+8)=*(int*)(bf.bufferBlock[bt].values+8)+1;
		NodeNumber=*(int*)(bf.bufferBlock[bt].values+8);
		*(int*)(node+12)=NodeNumber;
		*(int*)(node1+12)=NodeNumber;
		bf.writeBlock(bt);

		bt=bf.GiveMeABlock(name,NodeNumber);
		memcpy(bf.bufferBlock[bt].values,root,BSize);
		bf.writeBlock(bt);

		bt=bf.GiveMeABlock(name,0);
		*(int*)(bf.bufferBlock[bt].values+12)=NodeNumber;
		bf.writeBlock(bt);
	}



	//消除儿子上的多余数据
	if(nodetype==0){
		int x;
		brother=*(int*)(son+16+keylength[type]+4);
		x=0;
		temp=*(int*)(son+16+brother*l+keylength[type]+4);
		while(temp!=-1){
			if(type==0||type==1){
				*(int*)(son+16+brother*l)=*(int*)(son+16+temp*l);
			}
			else{
				memcpy(son+16+brother*l,son+16+temp*l,20);
			}
			x=brother;
			brother=*(int*)(son+16+brother*l+keylength[type]+4);
			temp=*(int*)(son+16+brother*l+keylength[type]+4);
		}
		*(int*)(son+16+brother*l+keylength[type]+4)=0;
		*(int*)(son+16+x*l+keylength[type]+4)=-1;

	}
	//写入所有修改的结点

	int bt=bf.GiveMeABlock(name,0);
	*(int*)(bf.bufferBlock[bt].values+8)=*(int*)(bf.bufferBlock[bt].values+8)+1;
	NodeNumber=*(int*)(bf.bufferBlock[bt].values+8);
	bf.writeBlock(bt);
	temp=0;
	brother=*(int*)(node1+16+keylength[type]+4);
	while(brother!=-1){
		int w=bf.GiveMeABlock(name,*(int*)(node1+16+brother*l+keylength[type]+8));
		*(int*)(bf.bufferBlock[w].values+12)=NodeNumber;
		bf.writeBlock(w);
		bf.flashBack(w);
		if(*(int*)(node1+16+brother*l+keylength[type]+4)==-1){
			w=bf.GiveMeABlock(name,*(int*)(node1+16+temp*l+keylength[type]+12));
			*(int*)(bf.bufferBlock[w].values+12)=NodeNumber;
			bf.writeBlock(w);
			bf.flashBack(w);
		}
		temp=brother;
		brother=*(int*)(node1+16+brother*l+keylength[type]+4);
	}


	int blockofnode=*(int*)(son+12);
	if(flag==2){
		*(int*)(son+12)=NodeNumber;
	}
	else if(flag==1){
		*(int*)(son+12)=NodeNumber;
		int y=bf.GiveMeABlock(name,son1);
		*(int*)(bf.bufferBlock[y].values+12)=NodeNumber;
		bf.writeBlock(y);
	}

	memcpy(bf.bufferBlock[bn].values,son,BSize);
	bf.writeBlock(bn);

	bn=bf.GiveMeABlock(name,blockofnode);
	memcpy(bf.bufferBlock[bn].values,node,BSize);
	bf.writeBlock(bn);



	bn=bf.GiveMeABlock(name,NodeNumber);
	memcpy(bf.bufferBlock[bn].values,node1,BSize);
	bf.writeBlock(bn);

	//寻找父亲以及递归完结条件

	bn=bf.GiveMeABlock(name,*(int*)(node+12));
	char *parent=new char[BSize];
	memcpy(parent,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);


	if(*(int*)(parent)<*(int*)(parent+4)){
		//在父亲上添一个元素
		insert_internal(parent,blockofnode,NodeNumber);
	}
	else{
		char *parent2=new char[BSize];
		split_internal(parent, parent2,blockofnode,NodeNumber);
		delete[] parent2;
	}


	delete[] parent;
	delete[] son;

}

void BpTree::insert_internal(char *node,int son1, int son2) {
	int t;
	int temp;
	int maxnum=*(int*)(node+4);
	int l=keylength[type]+16;
	*(int*)(node)=*(int*)(node)+1;


	//添加新元素，并存储最右边的元素ikey
	int ikey,tikey;
	float fkey,tfkey;
	char ckey[20],tckey[20];
	int bn=bf.GiveMeABlock(name,son2);
	char *son=new char[BSize];
	memcpy(son,bf.bufferBlock[bn].values,BSize);
	bf.useBlock(bn);
	int temp1=*(int*)(bf.bufferBlock[bn].values+16+keylength[type]+4);
	int nodetype=*(int*)(bf.bufferBlock[bn].values+8);

	int brother=*(int*)(node+16+keylength[type]+4);
	int a=0;
	while(brother!=-1){
		if(*(int*)(node+16+brother*l+keylength[type]+8)==son1){
			break;
		}
		a=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);
	}
	if(brother!=-1){
		if(type==0){
			ikey=*(int*)(node+16+brother*l);
			*(int*)(node+16+brother*l)=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else if(type==1){
			fkey=*(int*)(node+16+brother*l);
			*(int*)(node+16+brother*l)=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else{
			memcpy(ckey,node+16+brother*l,20);
			memcpy(node+16+brother*l,bf.bufferBlock[bn].values+16+temp1*l,20);
		}
		t=*(int*)(node+16+brother*l+keylength[type]+12);
		*(int*)(node+16+brother*l+keylength[type]+12)=son2;
		temp=brother;
		brother=*(int*)(node+16+brother*l+keylength[type]+4);


		while (brother!=-1){
			if(type==0){
				tikey=*(int*)(node+16+brother*l);
				*(int*)(node+16+brother*l)=ikey;
				ikey=tikey;
			}
			else if(type==1){
				tfkey=*(int*)(node+16+brother*l);
				*(int*)(node+16+brother*l)=fkey;
				fkey=tfkey;
			}
			else{
				memcpy(tckey,node+16+brother*l,20);
				memcpy(node+16+brother*l,ckey,20);
				memcpy(ckey,tckey,20);
			}
			int tt=*(int*)(node+16+brother*l+keylength[type]+12);
			*(int*)(node+16+brother*l+keylength[type]+12)=t;
			t=tt;
			*(int*)(node+16+brother*l+keylength[type]+8)=*(int*)(node+16+temp*l+keylength[type]+12);
			temp=brother;
			brother=*(int*)(node+16+brother*l+keylength[type]+4);
		}
	}
	else{
		t=son2;
		temp=a;
		if(type==0){
			ikey=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else if(type==1){
			fkey=*(int*)(bf.bufferBlock[bn].values+16+temp1*l);
		}
		else{
			memcpy(ckey,bf.bufferBlock[bn].values+16+temp1*l,20);
		}


	}
	int mostright=temp;
	//添加最右边的元素
	for(int i=1;i<=maxnum;i++){
		if(*(int*)(node+16+i*l+keylength[type]+4)==0){
			if(*(int*)(node)==1){
				*(int*)(node+16+i*l+keylength[type]+4)=-1;
				*(int*)(node+16+i*l+keylength[type]+8)=son1;
				*(int*)(node+16+i*l+keylength[type]+12)=son2;
				*(int*)(node+16+keylength[type]+4)=i;
				if(type==0){
					*(int*)(node+16+i*l)=ikey;
				}
				else if(type==1){
					*(int*)(node+16+i*l)=fkey;
				}
				else{
					memcpy(node+16+i*l,ckey,20);
				}
			}
			else{
				*(int*)(node+16+i*l+keylength[type]+4)=-1;
				*(int*)(node+16+i*l+keylength[type]+8)=*(int*)(node+16+temp*l+keylength[type]+12);
				*(int*)(node+16+i*l+keylength[type]+12)=t;
				*(int*)(node+16+mostright*l+keylength[type]+4)=i;
				if(type==0){
					*(int*)(node+16+i*l)=ikey;
				}
				else if(type==1){
					*(int*)(node+16+i*l)=fkey;
				}
				else{
					memcpy(node+16+i*l,ckey,20);
				}
			}

			break;
		}
	}

	//消除儿子上的多余数据
	if(nodetype==0){
		int x;
		brother=*(int*)(son+16+keylength[type]+4);
		x=brother;
		temp=*(int*)(son+16+brother*l+keylength[type]+4);
		while(temp!=-1){
			if(type==0||type==1){
				*(int*)(son+16+brother*l)=*(int*)(son+16+temp*l);
			}
			else{
				memcpy(son+16+brother*l,son+16+temp*l,20);
			}
			x=brother;
			brother=*(int*)(son+16+brother*l+keylength[type]+4);
			temp=*(int*)(son+16+brother*l+keylength[type]+4);
		}
		*(int*)(son+16+brother*l+keylength[type]+4)=0;
		*(int*)(son+16+x*l+keylength[type]+4)=-1;

	}

	memcpy(bf.bufferBlock[bn].values,son,BSize);
	bf.writeBlock(bn);

	bn=bf.GiveMeABlock(name,*(int*)(son+12));
	memcpy(bf.bufferBlock[bn].values,node,BSize);
	bf.writeBlock(bn);



}

void BpTree::Delete(Data *key) {}
int* BpTree::Range(Data *key1, Data *key2) {}


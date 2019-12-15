#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "str.h"

struct sysTables{
    char tablename[21];      //存放表名
	int attrcount;           //表中属性的数量
};

struct sysColumns{
    char tablename[21];  //表名       
	char attrname[21];   //属性名      
	AttrType attrtype;    //属性类型
	int attrlength;      //属性长度
	int attroffeset;     //属性在记录中的偏移量
	char ix_flag;        //该属性列上是否存在索引的标识,1表示存在，0表示不存在
	char indexname[21];   //索引名称
};

void ExecuteAndMessage(char * ,CEditArea*,CHustBaseDoc*);
bool CanButtonClick();

RC CreateDB(char *dbpath,char *dbname);
RC DropDB(char *dbname);
RC OpenDB(char *dbname);
RC CloseDB();

RC execute(char * sql, CHustBaseDoc*);

RC CreateTable(char *relName,int attrCount,AttrInfo *attributes);
RC DropTable(char *relName);
RC CreateIndex(char *indexName,char *relName,char *attrName);
RC DropIndex(char *indexName);
RC Insert(char *relName,int nValues,Value * values);
RC Delete(char *relName,int nConditions,Condition *conditions);
RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions);

#endif
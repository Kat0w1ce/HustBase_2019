#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "HustBaseDoc.h"

void ExecuteAndMessage(char * sql,CEditArea* editArea){//根据执行的语句类型在界面上显示执行结果。此函数需修改
	std::string s_sql = sql;
	if(s_sql.find("select") == 0){
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//将查询结果处理一下，整理成下面这种形式
		//调用editArea->ShowSelResult(col_num,row_num,fields,rows);
		int col_num = 5;
		int row_num = 3;
		char ** fields = new char *[5];
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],0,20);
			fields[i][0] = 'f';
			fields[i][1] = i+'0';
		}
		char *** rows = new char**[row_num];
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];
			for(int j = 0;j<col_num;j++){
				rows[i][j] = new char[20];
				memset(rows[i][j],0,20);
				rows[i][j][0] = 'r';
				rows[i][j][1] = i + '0';
				rows[i][j][2] = '+';
				rows[i][j][3] = j + '0';
			}
		}
		editArea->ShowSelResult(col_num,row_num,fields,rows);
		for(int i = 0;i<5;i++){
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	RC rc = execute(sql);
	int row_num = 0;
	char**messages;
	switch(rc){
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "功能未实现";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX
	
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////判断SQL语句为select语句

			//break;

			case 2:
			//判断SQL语句为insert语句

			case 3:	
			//判断SQL语句为update语句
			break;

			case 4:					
			//判断SQL语句为delete语句
			break;

			case 5:
			//判断SQL语句为createTable语句
			break;

			case 6:	
			//判断SQL语句为dropTable语句
			break;

			case 7:
			//判断SQL语句为createIndex语句
			break;
	
			case 8:	
			//判断SQL语句为dropIndex语句
			break;
			
			case 9:
			//判断为help语句，可以给出帮助提示
			break;
		
			case 10: 
			//判断为exit语句，可以由此进行退出操作
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//弹出警告框，sql语句词法解析错误信息
		return rc;
	}
}

RC CreateDB(char *dbpath,char *dbname){
	char *createPath = (char *)malloc(100);
	memset(createPath, 0, 100);
	strcat(createPath, dbpath);
	strcat(createPath, "\\");
	strcat(createPath, dbname);
	if (CreateDirectory(createPath, NULL))
	{
		SetCurrentDirectory(createPath);
		RC tempRc;
		tempRc = RM_CreateFile("SYSTABLES", 25);
		if (tempRc != SUCCESS)
		{
			free(createPath);
			return tempRc;
		}
		tempRc = RM_CreateFile("SYSCOLUMNS", 76);//使用记录文件
		if (tempRc != SUCCESS)
		{
			free(createPath);
			return tempRc;
		}
	}
	free(createPath);
	return SUCCESS;
}

void myDeleteFile(CString dir_path)
{  //delete file recursively
	CFileFind tempFind;
	CString path;
	path.Format("%s/*.*", dir_path);
	BOOL bWorking = tempFind.FindFile(path);
	while (bWorking)
	{
		bWorking = tempFind.FindNextFile();
		if (tempFind.IsDirectory() && !tempFind.IsDots()) 
		{
			myDeleteFile(tempFind.GetFilePath());
			RemoveDirectory(tempFind.GetFilePath());
		}
		else 
		{
			DeleteFile(tempFind.GetFilePath());
		}
	}

}

RC DropDB(char *dbname){
	myDeleteFile(dbname);
	if (RemoveDirectory(dbname))
	{
		AfxMessageBox("Success Delete Database!");
	}
	return SUCCESS;
}

RC OpenDB(char *dbname){
	if (SetCurrentDirectory(dbname))
	{
		return SUCCESS;
	}
	return SQL_SYNTAX;
}


RC CloseDB(){
	return SUCCESS;
}

//创建一个名为relName的表。参数attrCount表示关系中属性的数量（取值为1到MAXATTRS之间）。
//参数attributes是一个长度为attrCount的数组。对于新关系中第i个属性，
//attributes数组中的第i个元素包含名称、类型和属性的长度（见AttrInfo结构定义）。
RC CreateTable(char *relName, int attrCount, AttrInfo *attributes)
{
	RC tempRc;
	RM_FileHandle *columnHandle=NULL, *tableHandle=NULL;
	RID *tempRid=NULL;
	int recordSize = 0;
	//open systables and syscolumns
	tempRid = (RID *)malloc(sizeof(RID));
	columnHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tableHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	tempRid->bValid = false;
	columnHandle->bOpen = false;
	tableHandle->bOpen = false;

	tempRc = RM_OpenFile("SYSTABLES", tableHandle);
	if (tempRc != SUCCESS)
		return tempRc;
	tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
	if (tempRc != SUCCESS)
		return tempRc;
	//insert to systables and syscolumns
	char *pData = (char *)malloc(sizeof(sysTables));
	memcpy(pData, relName, 21);
	memcpy(pData + 21, &attrCount, sizeof(int));
	tempRc = InsertRec(tableHandle, pData, tempRid);
	if (tempRc != SUCCESS)
		return tempRc;
	tempRc = RM_CloseFile(tableHandle);
	if (tempRc != SUCCESS)
		return tempRc;
	free(tableHandle); 
	free(tempRid);
	free(pData);
	
	for (int i = 0, offset = 0; i < attrCount; i++)
	{
		pData = (char *)malloc(sizeof(sysColumns));
		memcpy(pData, relName, 21);
		memcpy(pData + 21, (attributes + i)->attrName, 21);
		memcpy(pData + 21 + 21, &((attributes + i)->attrType), sizeof(int));
		memcpy(pData + 42 + sizeof(int), &((attributes + i)->attrLength), sizeof(int));
		memcpy(pData + 42 + 2 * sizeof(int), &offset, sizeof(int));
		memcpy(pData + 42 + 3 * sizeof(int), "0", sizeof(char));
		tempRid = (RID*)malloc(sizeof(RID)); tempRid->bValid = false;
		tempRc = InsertRec(columnHandle, pData, tempRid);
		if (tempRc != SUCCESS)return tempRc;
		free(pData); free(tempRid);
		offset += (attributes + i)->attrLength;
	}
	tempRc = RM_CloseFile(columnHandle);
	if (tempRc != SUCCESS) return tempRc;
	free(columnHandle);

	for (int i = 0; i < attrCount; i++)
	{
		recordSize += (attributes + i)->attrLength;
	}
	tempRc = RM_CreateFile(relName, recordSize);
	if (tempRc != SUCCESS)return tempRc;
	return SUCCESS;
}

//销毁名为relName的表以及在该表上建立的所有索引。
RC DropTable(char *relName)
{
	RC tempRc;
	CFile tempFile;
	RM_FileHandle *columnHandle = NULL, *tableHandle = NULL;
	RM_FileScan *tempFileScan = NULL;
	RM_Record *tableRec = NULL, *columnRec = NULL;

	tableHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tableHandle->bOpen = false;
	columnHandle->bOpen = false;
	tableRec = (RM_Record*)malloc(sizeof(RM_Record));
	columnRec = (RM_Record*)malloc(sizeof(RM_Record));
	tableRec->bValid = false;
	columnRec->bValid = false;

	tempRc = RM_OpenFile("SYSTABLES", tableHandle);
	if (tempRc != SUCCESS) return tempRc;
	tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
	if (tempRc != SUCCESS)return tempRc;

	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;
	tempRc = OpenScan(tempFileScan, tableHandle, 0, NULL);
	if (tempRc != SUCCESS)return tempRc;
	//use getnextrec to find record
	while (GetNextRec(tempFileScan, tableRec) == SUCCESS)
	{
		if (!strcmp(relName, tableRec->pData))
		{
			DeleteRec(tableHandle, &(tableRec->rid));
			break;
		}
	}
	CloseScan(tempFileScan);
	tempFileScan->bOpen = false;
	tempRc = OpenScan(tempFileScan, columnHandle, 0, NULL);
	if (tempRc != SUCCESS)return tempRc;
	while (GetNextRec(tempFileScan, columnRec) == SUCCESS)
	{
		if (!strcmp(relName, columnRec->pData))
		{
			DeleteRec(columnHandle, &(columnRec->rid));
		}
	}
	CloseScan(tempFileScan);
	free(tempFileScan);
	tempRc = RM_CloseFile(tableHandle);
	if (tempRc != SUCCESS)return tempRc;
	tempRc = RM_CloseFile(columnHandle);
	if (tempRc != SUCCESS)return tempRc;
	free(tableHandle);
	free(columnHandle);
	free(tableRec);
	free(columnRec);
	DeleteFile((LPCTSTR)relName);
	return SUCCESS;
}

RC CreateIndex(char *indexName, char *relName, char *attrName)
{

}

bool CanButtonClick(){//需要重新实现
	//如果当前有数据库已经打开
	return true;
	//如果当前没有数据库打开
	//return false;
}

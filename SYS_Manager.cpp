#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "HustBaseDoc.h"

void ExecuteAndMessage(char *sql, CEditArea* editArea)
{//根据执行的语句类型在界面上显示执行结果。此函数需修改
	std::string s_sql = sql;
	RC rc;
	if (s_sql.find("select") == 0) {//是查询语句则执行以下，否则跳过
		SelResult res;
		Init_Result(&res);
		rc = Query(sql, &res);
		if (rc != SUCCESS)return;
		int col_num = res.col_num;//列
		int row_num = 0;//行
		SelResult *tmp = &res;
		while (tmp) {//所有节点的记录数之和
			row_num += tmp->row_num;
			tmp = tmp->next_res;
		}
		char ** fields = new char *[20];//各字段名称
		for (int i = 0; i<col_num; i++) {
			fields[i] = new char[20];
			memset(fields[i], '\0', 20);
			memcpy(fields[i], res.fields[i], 20);
		}
		tmp = &res;
		char *** rows = new char**[row_num];//结果集
		for (int i = 0; i<row_num; i++) {
			rows[i] = new char*[col_num];//存放一条记录
			for (int j = 0; j <col_num; j++)
			{
				rows[i][j] = new char[20];//一条记录的一个字段
				memset(rows[i][j], '\0', 20);
				memcpy(rows[i][j], tmp->res[i][j], 20);
			}
			if (i == 99)tmp = tmp->next_res;//每个链表节点最多记录100条记录
		}
		editArea->ShowSelResult(col_num, row_num, fields, rows);
		for (int i = 0; i<20; i++) {
			delete[] fields[i];
		}
		delete[] fields;
		Destory_Result(&res);
		return;
	}
	rc = execute(sql);//非查询语句则执行其他SQL语句，成功返回SUCCESS
	int row_num = 0;
	char**messages;
	switch (rc) {
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "功能未实现";
		editArea->ShowMessage(row_num, messages);
		delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;//声明
	RC rc, tempRc;
	sql_str = get_sqlstr();//初始化
	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX
	SelResult *res;
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
		case 1:
			////判断SQL语句为select语句
			break;

		case 2:
			//判断SQL语句为insert语句
			tempRc = Insert(sql_str->sstr.ins.relName, sql_str->sstr.ins.nValues, sql_str->sstr.ins.values);
			break;

		case 3:
			//判断SQL语句为update语句
			tempRc = Update(sql_str->sstr.upd.relName, sql_str->sstr.upd.attrName, &sql_str->sstr.upd.value, sql_str->sstr.upd.nConditions, sql_str->sstr.upd.conditions);
			break;

		case 4:
			//判断SQL语句为delete语句
			tempRc = Delete(sql_str->sstr.del.relName, sql_str->sstr.del.nConditions, sql_str->sstr.del.conditions);
			break;

		case 5:
			//判断SQL语句为createTable语句
			tempRc = CreateTable(sql_str->sstr.cret.relName, sql_str->sstr.cret.attrCount, sql_str->sstr.cret.attributes);
			break;
		case 6:
			//判断SQL语句为dropTable语句
			tempRc = DropTable(sql_str->sstr.drt.relName);
			break;

		case 7:
			//判断SQL语句为createIndex语句
			tempRc = CreateIndex(sql_str->sstr.crei.indexName, sql_str->sstr.crei.relName, sql_str->sstr.crei.attrName);
			break;

		case 8:
			//判断SQL语句为dropIndex语句
			tempRc = DropIndex(sql_str->sstr.dri.indexName);
			break;

		case 9:
			//判断为help语句，可以给出帮助提示
			break;
		case 10:
			//判断为exit语句，可以由此进行退出操作
			break;
		}
		return tempRc;
	}
	else {
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
	RM_FileHandle *columnHandle = NULL, *tableHandle = NULL;
	RID *tempRid = NULL;
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


//该函数在关系relName的属性attrName上创建名为indexName的索引。
//函数首先检查在标记属性上是否已经存在一个索引，如果存在，则返回一个非零的错误码。
//否则，创建该索引。创建索引的工作包括：
//①创建并打开索引文件；②逐个扫描被索引的记录，并向索引文件中插入索引项；③关闭索引。
RC CreateIndex(char *indexName, char *relName, char *attrName)
{
	RC tempRc;
	RM_FileHandle *columnHandle = NULL;
	RM_FileScan *tempFileScan = NULL;
	RM_Record *columnRec = NULL;

	columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	columnHandle->bOpen = false;
	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;

	tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
	if (tempRc != SUCCESS)
		return tempRc;
	/*
	typedef struct
	{
	int bLhsIsAttr, bRhsIsAttr;	//条件的左、右分别是属性（1）还是值（0）
	AttrType attrType;			//该条件中数据的类型
	int LattrLength, RattrLength;//若是属性的话，表示属性的长度
	int LattrOffset, RattrOffset;	//若是属性的话，表示属性的偏移量
	CompOp compOp;			//比较操作符
	void *Lvalue, *Rvalue;		//若是值的话，指向对应的值
	}Con;
	*/

	//build conditions
	Con *tempCon = NULL;
	tempCon = (Con*)malloc(sizeof(Con) * 2);
	(*tempCon).attrType = chars;
	(*tempCon).compOp = EQual;
	(*tempCon).bLhsIsAttr = 1;
	(*tempCon).LattrLength = 21;
	(*tempCon).LattrOffset = 0;
	(*tempCon).bRhsIsAttr = 0;
	(*tempCon).Rvalue = relName;

	(tempCon + 1)->attrType = chars;
	(tempCon + 1)->compOp = EQual;
	(tempCon + 1)->bLhsIsAttr = 1;
	(tempCon + 1)->LattrLength = 21;
	(tempCon + 1)->LattrOffset = 21;
	(tempCon + 1)->bRhsIsAttr = 0;
	(tempCon + 1)->Rvalue = attrName;

	OpenScan(tempFileScan, columnHandle, 2, tempCon);
	columnRec = (RM_Record*)malloc(sizeof(RM_Record));
	tempRc = GetNextRec(tempFileScan, columnRec);
	if (tempRc != SUCCESS)return tempRc;
	if (*(columnRec->pData + 42 + 3 * sizeof(int)) != '0')
	{
		return FAIL;
	}

	*(columnRec->pData + 42 + 3 * sizeof(int)) = '1';
	memset(columnRec->pData + 42 + 3 * sizeof(int) + sizeof(char), '\0', 21);
	memcpy(columnRec->pData + 42 + 3 * sizeof(int) + sizeof(char), indexName, strlen(indexName));
	UpdateRec(columnHandle, columnRec);

	RM_CloseFile(columnHandle); free(columnHandle);
	CloseScan(tempFileScan); free(tempFileScan);
	free(tempCon);

	AttrType *tempAttrType = NULL;
	tempAttrType = (AttrType*)malloc(sizeof(AttrType));
	memcpy(tempAttrType, columnRec->pData + 42, sizeof(int));

	int length;
	memcpy(&length, columnRec->pData + 42 + sizeof(int), sizeof(int));
	CreateIndex(indexName, *tempAttrType, length);
	free(tempAttrType);
	//open index file
	IX_IndexHandle *tempIndexHandle = NULL;
	tempIndexHandle = (IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
	tempIndexHandle->bOpen = false;
	OpenIndex(indexName, tempIndexHandle);
	//insert index to file
	RM_FileHandle *recFileHandle = NULL;
	RM_FileScan *recFileScan = NULL;
	RM_Record *rec = NULL;
	recFileHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	recFileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	rec = (RM_Record *)malloc(sizeof(RM_Record));
	recFileHandle->bOpen = false;
	recFileScan->bOpen = false;
	rec->bValid = false;

	RM_OpenFile(relName, recFileHandle);
	OpenScan(recFileScan, recFileHandle, 0, NULL);

	int attrOffset, attrLength;
	memcpy(&attrLength, columnRec->pData + 42 + sizeof(int), sizeof(int));
	memcpy(&attrOffset, columnRec->pData + 42 + 2 * sizeof(int), sizeof(int));
	free(columnRec);

	char *attrValue = NULL;
	attrValue = (char*)malloc(sizeof(char)*attrLength);

	//insert record to index file
	while (GetNextRec(recFileScan, rec) == SUCCESS)
	{
		memcpy(attrValue, rec->pData + attrOffset, attrLength);
		InsertEntry(tempIndexHandle, attrValue, &rec->rid);
	}

	CloseScan(recFileScan);
	RM_CloseFile(recFileHandle);
	CloseIndex(tempIndexHandle);

	free(tempIndexHandle);
	free(recFileHandle);
	free(recFileScan);
	free(attrValue);

	return SUCCESS;
}
//该函数用来删除名为indexName的索引。
//函数首先检查索引是否存在，如果不存在，则返回一个非零的错误码。
//否则，销毁该索引。
RC DropIndex(char *indexName)
{
	IX_IndexHandle *tempIndexHandle = NULL;
	RC tempRc;

	//judge the existence of index
	tempIndexHandle = (IX_IndexHandle*)malloc(sizeof(IX_IndexHandle));
	tempIndexHandle->bOpen = false;
	tempRc = OpenIndex(indexName, tempIndexHandle);
	if (tempRc != SUCCESS)return tempRc;
	CloseIndex(tempIndexHandle); free(tempIndexHandle);

	//delete index from table
	RM_FileHandle *columnHandle = NULL;
	RM_FileScan *tempFileScan = NULL;
	RM_Record *columnRec = NULL;

	columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	columnHandle->bOpen = false;
	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;

	tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
	if (tempRc != SUCCESS) return tempRc;
	//build conditions
	Con *tempCon = NULL;
	tempCon = (Con*)malloc(sizeof(Con));
	(*tempCon).attrType = chars;
	(*tempCon).compOp = EQual;
	(*tempCon).bLhsIsAttr = 1;
	(*tempCon).LattrLength = strlen(indexName) + 1;
	(*tempCon).LattrOffset = 42 + 3 * sizeof(int) + sizeof(char);
	(*tempCon).bRhsIsAttr = 0;
	(*tempCon).Rvalue = indexName;

	OpenScan(tempFileScan, columnHandle, 1, tempCon);
	columnRec = (RM_Record*)malloc(sizeof(RM_Record));
	tempRc = GetNextRec(tempFileScan, columnRec);

	if (tempRc == SUCCESS)
	{
		*(columnRec->pData + 42 + 3 * sizeof(int)) = '0';
		memset(columnRec->pData + 42 + 3 * sizeof(int) + sizeof(char), '\0', 21);
		UpdateRec(columnHandle, columnRec);
	}

	CloseScan(tempFileScan); free(tempFileScan);
	RM_CloseFile(columnHandle); free(columnHandle);
	free(tempCon);

	DeleteFile((LPCTSTR)indexName);

	return SUCCESS;
}

//该函数用来在relName表中插入具有指定属性值的新元组，nValues为属性值个数，values为对应的属性值数组。
//函数根据给定的属性值构建元组，调用记录管理模块的函数插入该元组，
//然后在该表的每个索引中为该元组创建合适的索引项。
RC Insert(char *relName, int nValues, Value * values)
{
	RC tempRc;
	RID *tempRid;
	RM_FileScan *tempFileScan;
	RM_Record *tableRec, *columnRec;
	RM_FileHandle *tableHandle, *columnHandle, *dataHandle;
	char *tableName = relName;

	Con *tempCon = (Con *)malloc(sizeof(Con));
	tempCon->bLhsIsAttr = 1;
	tempCon->bRhsIsAttr = 0;
	tempCon->attrType = chars;
	tempCon->LattrLength = 21;
	tempCon->LattrOffset = 0;
	tempCon->RattrOffset = 0;
	tempCon->RattrLength = 0;
	tempCon->compOp = EQual;
	tempCon->Rvalue = (void*)tableName;

	tableHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tableRec = (RM_Record*)malloc(sizeof(RM_Record));
	tableHandle->bOpen = false;
	tableRec->bValid = false;

	tempRc = RM_OpenFile("SYSTABLES", tableHandle);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Open System TablesFile Fail!");
		return tempRc;
	}
	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;
	tempRc = OpenScan(tempFileScan, tableHandle, 1, tempCon);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Scan System TablesFile Fail!");
		return tempRc;
	}
	tempRc = GetNextRec(tempFileScan, tableRec);
	if (tempRc == SUCCESS)
	{
		int attrCount;
		memcpy(&attrCount, tableRec->pData + 21, sizeof(int));
		CloseScan(tempFileScan); free(tempFileScan);
		if (attrCount == nValues)
		{
			int recordSize = 0;
			sysColumns *tmp, *column;
			columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
			columnHandle->bOpen = false;
			tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
			if (tempRc != SUCCESS)
			{
				AfxMessageBox("Open System ColumnsFile Fail!");
				return tempRc;
			}
			tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
			tempFileScan->bOpen = false;
			tempRc = OpenScan(tempFileScan, columnHandle, 0, NULL);
			if (tempRc != SUCCESS)
			{
				AfxMessageBox("Scan System ColumnsFile Fail!");
				return tempRc;
			}
			columnRec = (RM_Record*)malloc(sizeof(RM_Record));
			columnRec->bValid = false;

			int i = 0;
			column = (sysColumns*)malloc(sizeof(sysColumns));
			tmp = column;
			while (GetNextRec(tempFileScan, columnRec) == SUCCESS)
			{
				int attrLength;
				if (!strcmp(relName, columnRec->pData))
				{  //将对应table信息复制
					i++;
					memcpy(&attrLength, columnRec->pData + 46, sizeof(int));
					recordSize += attrLength;
					memcpy(tmp->tablename, columnRec->pData, 21);
					memcpy(tmp->attrname, columnRec->pData + 21, 21);
					memcpy(&(tmp->attrtype), columnRec->pData + 42, sizeof(int));
					memcpy(&(tmp->attrlength), columnRec->pData + 42 + sizeof(int), sizeof(int));
					memcpy(&(tmp->attroffeset), columnRec->pData + 42 + 2 * sizeof(int), sizeof(int));
					++tmp;
				}
				if (i == nValues)
					break;
			}
			tmp = column;
			CloseScan(tempFileScan);
			free(tempFileScan);
			dataHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
			dataHandle->bOpen = false;

			tempRc = RM_OpenFile(relName, dataHandle);
			if (tempRc != SUCCESS)
			{
				AfxMessageBox("Open Data Tables File Fail!");
				return tempRc;
			}
			char *pData = (char*)malloc(sizeof(recordSize));
			for (int i = 0; i < nValues; i++)
			{
				Value *localValue = values + nValues - i - 1;
				AttrType atype = localValue->type;
				if (atype != (tmp + i)->attrtype)
				{
					AfxMessageBox("Insert Syntax Error!");
					return SQL_SYNTAX;
				}
				memcpy(pData + (tmp + i)->attroffeset, localValue->data, (tmp + i)->attrlength);
			}
			tempRid = (RID*)malloc(sizeof(RID));
			tempRid->bValid = false;
			InsertRec(dataHandle, pData, tempRid);//insert data			

			tempRc = RM_CloseFile(tableHandle);
			if (tempRc != SUCCESS)
				return tempRc;
			tempRc = RM_CloseFile(dataHandle);
			if (tempRc != SUCCESS)
				return tempRc;

			free(tempRid);
			free(tableRec);
			free(columnRec);
			free(dataHandle);
			free(tableHandle);
		}
		else
		{
			AfxMessageBox("属性个数不相同，插入失败!");
			return tempRc;
		}
	}
	else
	{
		AfxMessageBox("Target Tables Is Not Exsit!");
		return tempRc;
	}
	return SUCCESS;
}

//该函数用来删除relName表中所有满足指定条件的元组以及该元组对应的索引项。
//如果没有指定条件，则此方法删除relName关系中所有元组。
//如果包含多个条件，则这些条件之间为与关系。
RC Delete(char *relName, int nConditions, Condition *conditions)
{
	RC tempRc;
	RM_FileHandle *tableHandle, *columnHandle, *dataHandle;
	RM_FileScan *tempFileScan;
	RM_Record *tableRec, *columnRec, *dataRec;

	Con *tempCon = (Con*)malloc(sizeof(Con));
	tempCon->Rvalue = (void*)relName;
	tempCon->bLhsIsAttr = 1;
	tempCon->bRhsIsAttr = 0;
	tempCon->attrType = chars;
	tempCon->compOp = EQual;
	tempCon->LattrLength = 21;
	tempCon->RattrLength = 0;
	tempCon->RattrOffset = 0;
	tempCon->LattrOffset = 0;

	tableHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tableHandle->bOpen = false;
	tempRc = RM_OpenFile("SYSTABLES", tableHandle);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Open Tables File Fail!");
		return tempRc;
	}
	tableRec = (RM_Record*)malloc(sizeof(RM_Record));
	tableRec->bValid = false;
	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;
	tempRc = OpenScan(tempFileScan, tableHandle, 1, NULL);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Open File Scan Fail!");
		return tempRc;
	}
	tempRc = GetNextRec(tempFileScan, tableRec);
	int nAttrCount;
	if (tempRc == SUCCESS)
	{
		memcpy(&nAttrCount, tableRec->pData + 21, sizeof(int));
		CloseScan(tempFileScan);
		free(tempFileScan);

		columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		columnHandle->bOpen = false;
		tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
		if (tempRc != SUCCESS)
		{
			AfxMessageBox("Open Column File Fail!");
			return tempRc;
		}
		tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
		tempFileScan->bOpen = false;
		columnRec = (RM_Record*)malloc(sizeof(RM_Record));
		columnRec->bValid = false;

		tempCon = (Con*)realloc(tempCon, sizeof(Con) * 2);
		(tempCon + 1)->bLhsIsAttr = 1;
		(tempCon + 1)->bRhsIsAttr = 0;
		(tempCon + 1)->attrType = chars;
		(tempCon + 1)->compOp = EQual;
		(tempCon + 1)->LattrLength = 21;
		(tempCon + 1)->RattrLength = 0;
		(tempCon + 1)->RattrOffset = 0;
		(tempCon + 1)->LattrOffset = 21;

		Con *tempCons = (Con *)malloc(nConditions * sizeof(Con));
		for (int i = 0; i < nConditions; i++)
		{
			if (conditions[i].bLhsIsAttr == 0 &&
				conditions[i].bRhsIsAttr == 1)
			{
				(tempCon + 1)->Rvalue = conditions[i].rhsAttr.attrName;
			}
			else if (conditions[i].bLhsIsAttr == 1 &&
				conditions[i].bRhsIsAttr == 0)
			{
				(tempCon + 1)->Rvalue = conditions[i].lhsAttr.attrName;
			}
			else
			{

			}

			OpenScan(tempFileScan, columnHandle, 2, tempCon);
			tempRc = GetNextRec(tempFileScan, columnRec);
			if (tempRc != SUCCESS)return tempRc;
			tempCons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
			tempCons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
			tempCons[i].compOp = conditions[i].op;
			if (conditions[i].bLhsIsAttr == 1)
			{        //left
				memcpy(&tempCons[i].LattrLength, columnRec->pData + 46, sizeof(int));
				memcpy(&tempCons[i].LattrOffset, columnRec->pData + 50, sizeof(int));
			}
			else
			{
				tempCons[i].attrType = conditions[i].lhsValue.type;
				tempCons[i].Lvalue = conditions[i].lhsValue.data;
			}

			if (conditions[i].bRhsIsAttr == 1)
			{       //right
				memcpy(&tempCons[i].RattrLength, columnRec->pData + 46, sizeof(int));
				memcpy(&tempCons[i].RattrOffset, columnRec->pData + 50, sizeof(int));
			}
			else
			{
				tempCons[i].attrType = conditions[i].rhsValue.type;
				tempCons[i].Rvalue = conditions[i].rhsValue.data;
			}
			CloseScan(tempFileScan);
		}
		free(tempFileScan);

		//Open Data File
		dataHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		dataHandle->bOpen = false;
		dataRec = (RM_Record*)malloc(sizeof(RM_Record));
		dataRec->bValid = false;
		tempRc = RM_OpenFile(relName, dataHandle);
		if (tempRc != SUCCESS)
		{
			AfxMessageBox("Open Data Table File Fail!");
			return tempRc;
		}
		tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
		tempRc=OpenScan(tempFileScan, dataHandle, nConditions, tempCons);
		if (tempRc != SUCCESS)
		{
			AfxMessageBox("Open Data File Scan Fail!");
			return tempRc;
		}
		while (GetNextRec(tempFileScan,dataRec)==SUCCESS)
		{
			DeleteRec(dataHandle, &dataRec->rid);
		}

		CloseScan(tempFileScan);
		free(tempFileScan);
		free(tempCons);

		RM_CloseFile(dataHandle);
		RM_CloseFile(tableHandle);
		RM_CloseFile(columnHandle);
		free(dataHandle);
		free(columnHandle);
		free(tableHandle);
		free(dataRec);
		free(tableRec);
		free(columnRec);
		free(tempCon);
		return SUCCESS;
	}
	else
	{
		AfxMessageBox("That Table Do Not Exsit!");
		RM_CloseFile(tableHandle);
		free(tableHandle);
		CloseScan(tempFileScan);
		free(tempFileScan);
		free(tableRec);
		return tempRc;
	}
	return SUCCESS;
}


//该函数用于更新relName表中所有满足指定条件的元组，在每一个更新的元组中将属性attrName的值设置为一个新的值。
//如果没有指定条件，则此方法更新relName中所有元组。
//如果要更新一个被索引的属性，应当先删除每个被更新元组对应的索引条目，然后插入一个新的索引条目。
RC Update(char *relName, char *attrName, Value *value, int nConditions, Condition *conditions)
{
	RC tempRc;
	RM_FileHandle *tableHandle, *columnHandle, *dataHandle;
	RM_FileScan *tempFileScan;
	RM_Record *tableRec, *columnRec, *dataRec;

	Con *tempCon = (Con*)malloc(sizeof(Con));
	tempCon->attrType = chars;
	tempCon->bLhsIsAttr = 1;
	tempCon->bRhsIsAttr = 0;
	tempCon->LattrLength = 21;
	tempCon->LattrOffset = 0;
	tempCon->RattrLength = 0;
	tempCon->RattrOffset = 0;
	tempCon->compOp = EQual;
	tempCon->Rvalue = (void*)relName;
	
	tableHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
	tableHandle->bOpen = false;
	tempRc = RM_OpenFile("SYSTABLES", tableHandle);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Open Table File Fail!");
		return tempRc;
	}
	tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
	tempFileScan->bOpen = false;
	tempRc = OpenScan(tempFileScan, tableHandle, 1, tempCon);
	if (tempRc != SUCCESS)
	{
		AfxMessageBox("Open File Scan Fail!");
		return tempRc;
	}
	tableRec = (RM_Record*)malloc(sizeof(RM_Record));
	tableRec->bValid = false;
	tempRc = GetNextRec(tempFileScan, tableRec);
	int nAttrCount;
	if (tempRc == SUCCESS)
	{
		memcpy(&nAttrCount, tableRec->pData + 21, sizeof(int));
		CloseScan(tempFileScan);
		free(tempFileScan);

		columnHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		columnHandle->bOpen = false;
		tempRc = RM_OpenFile("SYSCOLUMNS", columnHandle);
		if (tempRc != SUCCESS)
		{
			AfxMessageBox("Open Column File Fail!");
			return tempRc;
		}
		tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
		columnRec = (RM_Record*)malloc(sizeof(RM_Record));

		tempCon = (Con*)realloc(tempCon, sizeof(Con) * 2);
		tempCon->attrType = chars;
		tempCon->bLhsIsAttr = 1;
		tempCon->bRhsIsAttr = 0;
		tempCon->LattrLength = 21;
		tempCon->LattrOffset = 21;
		tempCon->RattrLength = 0;
		tempCon->RattrOffset = 0;
		tempCon->compOp = EQual;
		tempCon->Rvalue = (void*)attrName;

		tempRc = OpenScan(tempFileScan, columnHandle, 2, tempCon);
		if (tempRc != SUCCESS)
		{
			AfxMessageBox("Open File Scan Fail!");
			return tempRc;
		}
		tempRc = GetNextRec(tempFileScan, columnRec);
		if (tempRc == SUCCESS)
		{
			AttrType tempAttrType;
			memcpy(&tempAttrType, columnRec->pData + 42, sizeof(AttrType));
			int attrLength, attrOffset;
			memcpy(&attrLength, columnRec->pData + 46, sizeof(int));
			memcpy(&attrOffset, columnRec->pData + 50, sizeof(int));
			CloseScan(tempFileScan);

			Con *tempCons = (Con *)malloc(sizeof(Con)*nConditions);
			for (int i = 0; i < nConditions; i++)
			{
				if (conditions[i].bLhsIsAttr == 0 &&
					conditions[i].bRhsIsAttr == 1)
				{
					(tempCon + 1)->Rvalue = conditions[i].rhsAttr.attrName;
				}
				else if (conditions[i].bLhsIsAttr == 1 &&
					conditions[i].bRhsIsAttr == 0)
				{
					(tempCon + 1)->Rvalue = conditions[i].lhsAttr.attrName;
				}
				else
				{

				}

				OpenScan(tempFileScan, columnHandle, 2, tempCon);
				tempRc = GetNextRec(tempFileScan, columnRec);
				if (tempRc != SUCCESS)return tempRc;
				tempCons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
				tempCons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
				tempCons[i].compOp = conditions[i].op;
				if (conditions[i].bLhsIsAttr == 1)
				{        //left
					memcpy(&tempCons[i].LattrLength, columnRec->pData + 46, sizeof(int));
					memcpy(&tempCons[i].LattrOffset, columnRec->pData + 50, sizeof(int));
				}
				else
				{
					tempCons[i].attrType = conditions[i].lhsValue.type;
					tempCons[i].Lvalue = conditions[i].lhsValue.data;
				}

				if (conditions[i].bRhsIsAttr == 1)
				{       //right
					memcpy(&tempCons[i].RattrLength, columnRec->pData + 46, sizeof(int));
					memcpy(&tempCons[i].RattrOffset, columnRec->pData + 50, sizeof(int));
				}
				else
				{
					tempCons[i].attrType = conditions[i].rhsValue.type;
					tempCons[i].Rvalue = conditions[i].rhsValue.data;
				}
				CloseScan(tempFileScan);
			}
			free(tempFileScan);

			dataHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
			dataHandle->bOpen = false;
			dataRec = (RM_Record*)malloc(sizeof(RM_Record));
			dataRec->bValid = false;
			tempRc = RM_OpenFile(relName, dataHandle);
			if (tempRc != SUCCESS)
			{
				AfxMessageBox("Open Data File Fail!");
				return tempRc;
			}
			tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
			tempRc = OpenScan(tempFileScan, dataHandle, nConditions, tempCons);
			if (tempRc != SUCCESS)
			{
				AfxMessageBox("Open Data File Scan Fail!");
				return tempRc;
			}
			while (GetNextRec(tempFileScan,dataRec)==SUCCESS)
			{
				memcpy(dataRec->pData + attrOffset, value->data, attrLength);
				UpdateRec(tempFileScan->pRMFileHandle, dataRec);
			}
			CloseScan(tempFileScan);
			free(tempFileScan);
			free(tempCons);
		}
		else
		{
			AfxMessageBox("That Table Do Not Exsit This Property!");
			return tempRc;
		}
		RM_CloseFile(tableHandle);
		RM_CloseFile(dataHandle);
		RM_CloseFile(columnHandle);
		free(tableHandle);
		free(columnHandle);
		free(dataHandle);
		free(tempCon);
		free(tableRec);
		free(columnRec);
		free(dataRec);
		return SUCCESS;
	}
	else
	{
		AfxMessageBox("That Table Do Not Exsit!");
		RM_CloseFile(tableHandle);
		CloseScan(tempFileScan);
		free(tableHandle);
		free(tempFileScan);
		free(tableRec);
		return tempRc;
	}
	return SUCCESS;
}

bool CanButtonClick(){//需要重新实现
	//如果当前有数据库已经打开
	return true;
	//如果当前没有数据库打开
	//return false;
}

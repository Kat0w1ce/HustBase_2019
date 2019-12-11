#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "HustBaseDoc.h"

void ExecuteAndMessage(char * sql,CEditArea* editArea){//����ִ�е���������ڽ�������ʾִ�н�����˺������޸�
	std::string s_sql = sql;
	if(s_sql.find("select") == 0){
		SelResult res;
		Init_Result(&res);
		//rc = Query(sql,&res);
		//����ѯ�������һ�£����������������ʽ
		//����editArea->ShowSelResult(col_num,row_num,fields,rows);
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
		messages[0] = "�����ɹ�";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "���﷨����";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "����δʵ��";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX
	
	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////�ж�SQL���Ϊselect���

			//break;

			case 2:
			//�ж�SQL���Ϊinsert���

			case 3:	
			//�ж�SQL���Ϊupdate���
			break;

			case 4:					
			//�ж�SQL���Ϊdelete���
			break;

			case 5:
			//�ж�SQL���ΪcreateTable���
			break;

			case 6:	
			//�ж�SQL���ΪdropTable���
			break;

			case 7:
			//�ж�SQL���ΪcreateIndex���
			break;
	
			case 8:	
			//�ж�SQL���ΪdropIndex���
			break;
			
			case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ
			break;
		
			case 10: 
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
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
		tempRc = RM_CreateFile("SYSCOLUMNS", 76);//ʹ�ü�¼�ļ�
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

//����һ����ΪrelName�ı�����attrCount��ʾ��ϵ�����Ե�������ȡֵΪ1��MAXATTRS֮�䣩��
//����attributes��һ������ΪattrCount�����顣�����¹�ϵ�е�i�����ԣ�
//attributes�����еĵ�i��Ԫ�ذ������ơ����ͺ����Եĳ��ȣ���AttrInfo�ṹ���壩��
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

//������ΪrelName�ı��Լ��ڸñ��Ͻ���������������
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

//�ú����ڹ�ϵrelName������attrName�ϴ�����ΪindexName��������
//�������ȼ���ڱ���������Ƿ��Ѿ�����һ��������������ڣ��򷵻�һ������Ĵ����롣
//���򣬴��������������������Ĺ���������
//�ٴ������������ļ��������ɨ�豻�����ļ�¼�����������ļ��в���������۹ر�������
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
		int bLhsIsAttr, bRhsIsAttr;	//���������ҷֱ������ԣ�1������ֵ��0��
		AttrType attrType;			//�����������ݵ�����
		int LattrLength, RattrLength;//�������ԵĻ�����ʾ���Եĳ���
		int LattrOffset, RattrOffset;	//�������ԵĻ�����ʾ���Ե�ƫ����
		CompOp compOp;			//�Ƚϲ�����
		void *Lvalue, *Rvalue;		//����ֵ�Ļ���ָ���Ӧ��ֵ
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

	(tempCon+1)->attrType = chars;
	(tempCon+1)->compOp = EQual;
	(tempCon+1)->bLhsIsAttr = 1;
	(tempCon+1)->LattrLength = 21;
	(tempCon+1)->LattrOffset =21;
	(tempCon+1)->bRhsIsAttr = 0;
	(tempCon+1)->Rvalue = attrName;

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
	memcpy(columnRec->pData + 42 + 3 * sizeof(int) + sizeof(char), indexName,strlen(indexName));
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
	while (GetNextRec(recFileScan,rec)==SUCCESS)
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

bool CanButtonClick(){//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;
}

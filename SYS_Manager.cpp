#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "HustBaseDoc.h"

std::vector<RM_FileHandle *> vecHandle;

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
		if (RM_CreateFile("SYSTABLES", 25) == SUCCESS &&
			RM_CreateFile("SYSCOLUMNS", 76) == SUCCESS)
		{
			free(createPath);
			return SUCCESS;
		}
	}
	free(createPath);
	return SQL_SYNTAX;
}

RC DropDB(char *dbname){
	
	CFileFind tempFind;
	char *dropPath = (char *)malloc(300);
	memset(dropPath, 0, 300);
	strcat(dropPath, dbname);
	strcat(dropPath, "\\*.*");
	BOOL isFind = tempFind.FindFile(dropPath);
	if (isFind)
	{
		memset(dropPath, 0, 300);
		isFind = tempFind.FindNextFile();
		if ((!tempFind.IsDots()) && (!tempFind.IsDirectory()))
		{
			strcat(dropPath, dbname);
			strcat(dropPath, "\\");
			strcat(dropPath, tempFind.GetFileName().GetBuffer(200));
			DeleteFile(dropPath);
		}
	}
	tempFind.Close();
	free(dropPath);
	if (RemoveDirectory(dbname))
	{
		return SUCCESS;
	}
	return SQL_SYNTAX;
}

RC OpenDB(char *dbname){
	if (SetCurrentDirectory(dbname))
	{
		return SUCCESS;
	}
	return SQL_SYNTAX;
}


RC CloseDB(){
	for (std::vector<RM_FileHandle *>::size_type i = 0; i < vecHandle.size(); i++)
	{
		if (vecHandle[i] != NULL)
		{
			RM_CloseFile(vecHandle[i]);
		}
	}
	vecHandle.clear();
	return SUCCESS;
}

bool CanButtonClick(){//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;
}

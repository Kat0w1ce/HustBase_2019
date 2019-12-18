#ifndef __QUERY_MANAGER_H_
#define __QUERY_MANAGER_H_
#include "str.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"

typedef struct SelResult{
	int col_num;
	int row_num;
	AttrType type[20];	//��������ֶε���������
	int offset[20];      //ƫ��
	int length[20];		//��������ֶ�ֵ�ĳ���
	char fields[20][20];//����ʮ���ֶ���������ÿ���ֶεĳ��Ȳ�����20
	char ** res[100];	//���һ������¼
	SelResult * next_res;
}SelResult;

void Init_Result(SelResult * res);
void Destory_Result(SelResult * res);


RC tableExist(int nRelations, char **relations);

RC Query(char * sql,SelResult * res);
RC Select(int nSelAttrs,RelAttr **selAttrs,int nRelations,char **relations,int nConditions,Condition *conditions,SelResult * res);
RC singleNoCondition(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);
RC singleWithCondition(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);

#endif
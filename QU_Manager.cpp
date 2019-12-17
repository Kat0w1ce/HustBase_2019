#include "StdAfx.h"
#include "QU_Manager.h"

void Init_Result(SelResult * res) {
	res->next_res = NULL;
}

void Destory_Result(SelResult * res) {
	for (int i = 0; i < res->row_num; i++) {
		for (int j = 0; j < res->col_num; j++) {
			delete[] res->res[i][j];
		}
		delete[] res->res[i];
	}
	if (res->next_res != NULL) {
		Destory_Result(res->next_res);
	}
}

RC Select(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res)
{
	return SUCCESS;
}


RC singleNoCondition(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res)
{
	if (false)
	{  //has index

	}
	else
	{
		RC tempRc;
		SelResult *resHead = res;
		RM_FileHandle *tempFileHandle = NULL;
		RM_FileScan *tempFileScan = NULL;
		RM_Record *tempRec = NULL;
		Con tempCon[2];

		tempCon[0].attrType = chars;
		tempCon[0].bLhsIsAttr = 1;
		tempCon[0].LattrOffset = 0;
		tempCon[0].LattrLength = strlen(*relations) + 1;
		tempCon[0].compOp = EQual;
		tempCon[0].bRhsIsAttr = 0;
		tempCon[0].Rvalue = *relations;

		if (nSelAttrs == 1 && !strcmp((*selAttrs)->attrName, "*"))
		{ //select all
			tempFileHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
			tempFileHandle->bOpen = false;
			tempRc = RM_OpenFile("SYSTABLES", tempFileHandle);
			if (tempRc != SUCCESS) return tempRc;

			tempFileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
			tempFileScan->bOpen = false;
			tempRec = (RM_Record *)malloc(sizeof(RM_Record));
			tempRec->bValid = false;

			OpenScan(tempFileScan, tempFileHandle, 1, tempCon);
			tempRc = GetNextRec(tempFileScan, tempRec);
			if (tempRc != SUCCESS) return tempRc;

			//get num of table's property
			sysTables *table = (sysTables *)tempRec->pData;
			memcpy(&(resHead->col_num), tempRec->pData + 21, sizeof(int));

			CloseScan(tempFileScan);
			RM_CloseFile(tempFileHandle);

			//get type of property
			tempRc = RM_OpenFile("SYSCOLUMNS", tempFileHandle);
			if (tempRc != SUCCESS) return tempRc;

			OpenScan(tempFileScan, tempFileHandle, 1, tempCon);

			for (int i = 0; i < resHead->col_num; i++)
			{
				tempRc = GetNextRec(tempFileScan, tempRec);
				if (tempRc != SUCCESS) return tempRc;

				char * column = tempRec->pData;
				memcpy(&resHead->type[i], column + 42, sizeof(int));
				memcpy(&resHead->fields[i], column + 21, 21);
				memcpy(&resHead->length[i], column + 46, sizeof(int));
				memcpy(&resHead->offset, column + 50, sizeof(int));

			}
			CloseScan(tempFileScan);
		}
		else
		{ //not select all
			resHead->col_num = nSelAttrs;  //set column of result
			tempFileHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
			tempFileHandle->bOpen = false;
			tempFileScan = (RM_FileScan*)malloc(sizeof(RM_FileScan));
			tempFileScan->bOpen = false;
			tempRec = (RM_Record*)malloc(sizeof(RM_Record));
			tempRec->bValid = false;

			tempCon[1].attrType = chars;
			tempCon[1].bLhsIsAttr = 1;
			tempCon[1].LattrOffset = 21;
			tempCon[1].LattrLength = 21;
			tempCon[1].compOp = EQual;
			tempCon[1].bRhsIsAttr = 0;

			tempRc = RM_OpenFile("SYSCOLUMNS", tempFileHandle);
			if (tempRc != SUCCESS)return tempRc;

			//get information of property
			for (int i = 0; i < resHead->col_num; i++)
			{
				tempCon[1].Rvalue = (selAttrs[i])->attrName;//get name of property
				OpenScan(tempFileScan, tempFileHandle, 2, tempCon);
				tempRc = GetNextRec(tempFileScan, tempRec);
				if (tempRc != SUCCESS)return tempRc;

				char *column = tempRec->pData;
				
				memcpy(&resHead->fields[i], column + 21, 21);
				memcpy(&resHead->type[i], column + 42, sizeof(int));
				memcpy(&resHead->length[i], column + 46, sizeof(int));
				memcpy(&resHead->offset[i], column + 50, sizeof(int));
				CloseScan(tempFileScan);
			}
		}

		//scan table file to find record
		RM_CloseFile(tempFileHandle);
		free(tempFileHandle);
		tempFileHandle = (RM_FileHandle*)malloc(sizeof(RM_FileHandle));
		tempFileHandle->bOpen = false;
		tempRc = RM_OpenFile(*relations, tempFileHandle);
		if (tempRc != SUCCESS) return tempRc;

		OpenScan(tempFileScan, tempFileHandle, 0, NULL);

		int i = 0;
		resHead->row_num = 0;
		SelResult *curRes = resHead;
		while (GetNextRec(tempFileScan, tempRec) == SUCCESS)
		{
			if (curRes->row_num >= 100) //100 records per node
			{ //build new node
				curRes->next_res = (SelResult *)malloc(sizeof(SelResult));
				curRes->next_res->col_num = curRes->col_num;
				for (int j = 0; j < curRes->col_num; j++)
				{
					strncpy(curRes->next_res->fields[i], curRes->fields[i], strlen(curRes->fields[i]));
					curRes->next_res->type[i] = curRes->type[i];
					curRes->next_res->offset[i] = curRes->offset[i];
				}

				curRes = curRes->next_res;
				curRes->next_res = NULL;
				curRes->row_num = 0;
			}

			curRes->res[curRes->row_num] = (char **)malloc(sizeof(char *));
			*(curRes->res[curRes->row_num++]) = tempRec->pData;
		}

		CloseScan(tempFileScan);
		free(tempFileScan);
		RM_CloseFile(tempFileHandle);
		free(tempFileHandle);
		free(tempRec);
	}
	return SUCCESS;
}

RC Query(char * sql,SelResult * res){
	return SUCCESS;
}
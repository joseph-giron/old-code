// ��ʽ��ֵdll.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "windows.h"
#include "stdlib.h"


//�������ܣ�������������ߵ��������ý���ı��滻����ʽ
void CalRep(char *s)
{
	char * p;//���ƶ���ָ��
	char * pl,//ָ����ʽ�ַ�������߽�
		 * pr;//ָ����ʽ�ַ������ұ߽�
	int result;//��Ž��
	char szBuf[512];
	int left,right;//�����������ߵ���
	//ȡ��ߵ���ֵ
	p=s-1;
a:	if(((*p)==' ')||((*p)=='\t'))//���Կո����Ʊ��
	{
		p--;
		goto a;
	}
	while(1)
	{
		if(((*p)>='0')&&((*p)<='9'))	//����ַ���'0','9'֮��
		{
			p--;
		}
		else
			break;
	}
	left=atoi(++p);
	//ȡ��߽�
	pl=p;
	//ȡ�ұߵ���ֵ
	right=atoi(s+1);
	//ȡ�ұ߽�
	p=s+1;
b:	if(((*p)==' ')||((*p)=='\t'))//���Կո����Ʊ��
	{
		p++;
		goto b;
	}
	while(1)
	{
		if(((*p)>='0')&&((*p)<='9'))	//����ַ���'0','9'֮��
		{
			p++;
		}
		else
			break;
	}
	pr=--p;
	switch(*s)
	{
	case '+':
		result=left+right;
		break;
	case '-':
		result=left-right;
		break;
	case '*':
		result=left*right;
		break;
	case '/':
		result=left/right;
		break;
	}
	wsprintf(szBuf,"%d",result);
	lstrcpy(pl,szBuf);
	lstrcpy(szBuf,pr+1);//��������ʽ�ұߵ��ַ���
	lstrcat(pl,szBuf);
}

__declspec(dllexport) BOOL CALLBACK calculate(char *equation,char *result)
{
	char str[1024];//����ַ���
	char level[4]={'*','/','+','-'};//�������㼶
	int i;//���������
	int j;//�ַ�������

	lstrcpy(str,equation);
	for(i=0;i<sizeof(level);i++)
	{
		for(j=0;str[j]!=0;)
		{
			if(str[j]==level[i])
			{
				CalRep(&str[j]);
				continue;
			}
			j++;
		}
	}
	lstrcpy(result,str);
	return 1;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//��Ϊ���ش���ʱ�����Ľṹ����֪��unicodestring�ṹ���Ǻ�
//�����ض���һ��
typedef struct winstring
{
	int length;
	int max;
	TCHAR *buf;
}WINUNICODESTRING,*PWINUNICODESTRING;

//�ַ�������
int CopyWinUnicodeString(wchar_t* d,wchar_t* s,int max)
{
	int  i=0;
	while(1)
	{
		d[i]=s[i];
		if(!d[i]){break;}
		i++;
		max--;
		if(!max)
		{
			d[i]=0;
		}
	}
	return i;
}
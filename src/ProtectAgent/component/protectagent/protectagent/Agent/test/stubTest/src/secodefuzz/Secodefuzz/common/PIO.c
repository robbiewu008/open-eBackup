/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ������ṩ��д�ļ�����
����ϵͳ����

����Ҫ��ʼ��������������


*/
#include "PCommon.h"

#ifdef HAS_IO

// windows��ʱ��֧��Ŀ¼�������Ժ���˵
#ifndef _WIN32
#include <dirent.h>
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_IO

// windows��ʱ��֧��Ŀ¼�������Ժ���˵
#ifndef _WIN32
DIR * dir;
int OpenDir(char* dirPath) 
{
    dir = opendir(dirPath);

    if (NULL == dir)
    {
        hw_printf("opendir %s is erro\r\n",dirPath);
        return ENUM_NO;
    }
    return ENUM_YES;
}

char* ReadDir(void) 
{
    struct dirent * ptr;
    ptr = readdir(dir);

    if (ptr == NULL)
        return NULL;
    
    return ptr->d_name;
}

void CloseDir(void) 
{
    closedir(dir);
}
#else

int OpenDir(char* dirPath) 
{
    return ENUM_NO;
}

char* ReadDir(void) 
{
    return NULL;
}

void CloseDir(void) 
{

}


#endif

void ReadFromFile(char** data, int *len, char *path) 
{
    FILE *fp;  
    int fileSize;  

    if ((fp = fopen(path, "rb"))==NULL)
    {   
        *len = 0;
        *data = NULL;
        return;  
    }  

    fseek(fp, 0, SEEK_END);   
    fileSize = ftell(fp);  

    *data = (char *)HwMalloc(fileSize + 1);  
    (*data)[fileSize] = 0;

    // rewind(fp); 
    fseek(fp, 0L, SEEK_SET); 
    fread(*data, 1, fileSize, fp);
    fclose(fp);  

    // ��Ҫ������ʵ���ļ�����
    *len = fileSize;
    return; 
}

void WriteToFile(char* data, int len, char *path) 
{
    // Use raw C interface because this function may be called from a sig handler.
    FILE *out = fopen(path, "w");
    if (!out)
    {
        return;
    }
    fwrite(data, len, 1, out);
    fclose(out);
}

void WriteToFileFail(char* data, int len, char *path) 
{
    // Use raw C interface because this function may be called from a sig handler.
    FILE *out = fopen(path, "a+");
    if (!out)
    {
        return;
    }
    fprintf(out, "%s", data);
    fclose(out);
}

void RemoveFile(char *path) 
{
    if (remove(path) !=0) 
    {
        hw_printf("\t delete %s file failed,!!!!\n", path);
    }
}

void RenameFile(char *path ,char *newpath) 
{
    rename(path, newpath);
}

#else

int OpenDir(char* dirPath) 
{
    return ENUM_NO;
}

char* ReadDir(void) 
{
    return NULL;
}

void CloseDir(void) 
{

}


// �����֧�֣������ṩ�ĺ�����������
void WriteToFile(char* data, int len, char *path) 
{
    return;  
}
void WriteToFileFail(char* data, int len, char *path) 
{
    return; 
}
void ReadFromFile(char** data, int *len, char *path) 
{
    *len = 0;
    *data = NULL;
    return;  
}

void RemoveFile(char *path) 
{
    return; 
}

void RenameFile(char *path ,char *newpath) 
{
    return;
}

#endif

#ifdef __cplusplus
}
#endif

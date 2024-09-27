/*
版权所有 (c) 华为技术有限公司 2012-2018


file发包器，将最后变异的数据打印到文件里

//打印到一个文件里，每次覆盖
<Publisher class="file" name="writer">
    <Param name="FileName" value="fuzzed.jpg" />  
</Publisher>

//打印到多个文件里，fuzzed{1}.jpg，fuzzed{2}.jpg，fuzzed{3}.jpg  ...
<Publisher class="file" name="writer">
    <Param name="FileName" value="fuzzed{0}.jpg" />  
</Publisher>

*/
#ifndef Open_source

#include "../../XML.h"

#include<stdio.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static FILE *out[MAX_PITS_PARA_NUMBER];
static int isOpen[MAX_PITS_PARA_NUMBER] = {0};
static int g_count = 1;

static int PublisherFileOpen(char* name, SStateElement* tempAction)
{
    int id = GetPublisherId_ByName(name, tempAction);

    if (id >= MAX_PITS_PARA_NUMBER)
    {   
        xml_assert(0, "publisher not found by name");
    }

    if (isOpen[id] == 1)
    {   
        return ENUM_YES;
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherFileOpen----\r\n"); 
    }

    isOpen[id] = 1;

    char *para_value = GetPublisherParaValue_ByName(id, "FileName", tempAction);
    char *rel = MyMemmem(para_value, strlen(para_value), "{0}", 3);

    char buf1[256];
    char buf2[600];
    char buf3[256];

    if (rel == NULL)
    {
        memcpy(buf2, para_value, strlen(para_value) + 1);
    }
    else
    {
        buf1[0] = 0;
        buf2[0] = 0;
        buf3[0] = 0;

        memcpy(buf1, para_value, strlen(para_value) + 1);
        buf1[rel - para_value] = 0;

        Inltoa(g_count++, buf3, 10);
        sprintf(buf2, "%s{%s}%s", buf1, buf3, rel + 3);  // hw_sprintf
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s open file %s----\r\n", name, buf2); 
    }

    out[id] = fopen(buf2, "w");
    if (!out[id]) 
    { 
        xml_assert(0, "open file error");
    }

    return ENUM_YES;
}


static int PublisherFileOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherFileOutput----\r\n"); 
    }

    int id = GetPublisherId_ByName(name, tempAction);

    if (id >= MAX_PITS_PARA_NUMBER)
    {   
        xml_assert(0, "publisher not found by name");
    }

    if (isOpen[id] == 0)
    {   
        xml_assert(0, "publisher not open");
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    fwrite(buf, length, 1, out[id]);
    return ENUM_YES;
}

static int PublisherFileClose(char* name, SStateElement* tempAction)
{
    int id = GetPublisherId_ByName(name, tempAction);

    if (id >= MAX_PITS_PARA_NUMBER)
    {   
        xml_assert(0, "publisher not found by name");
    }

    if (isOpen[id] == 0)
    {   
        return ENUM_YES;
    }

    if(g_onOffDebugPublisher)
    {   
        printf("	----call PublisherFileClose----\r\n"); 
    }

    isOpen[id] = 0;
    fclose(out[id]);
    
    return ENUM_YES;
}

static struct PublisherGroup g_publisherFile = {
    "file",
    0xffffffff,
    PublisherFileOpen,
    PublisherFileOutput,
    PublisherFileClose
};

void InitPublisherFile(void)
{
    RegisterPublisherGroup(&g_publisherFile);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source

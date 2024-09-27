/*
版权所有 (c) 华为技术有限公司 2012-2018

对外依赖函数
*/
#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

hw_xmlDocPtr HW1xmlReadFile(const char *fileName, const char *encoding, int options)
{
#ifdef USE_libxm2lib
    return xmlParseFile(fileName); 
#else
    return hw_xmlReadFile(fileName, encoding, options);
#endif
}

hw_xmlNodePtr HW1xmlDocGetRootElement(hw_xmlDocPtr doc)
{
#ifdef USE_libxm2lib
    return xmlDocGetRootElement(doc);
#else
    return hw_xmlDocGetRootElement(doc);
#endif
}

hw_xmlChar *HW1xmlGetProp(hw_xmlNodePtr node, const hw_xmlChar *name)
{
#ifdef USE_libxm2lib
    return xmlGetProp(node, name);
#else
    return hw_xmlGetProp(node, name);
#endif
}

hw_xmlDocPtr HW1xmlParseMemory(const char *buffer, int size) 
{
#ifdef USE_libxm2lib
    return xmlParseMemory(buffer, size);
#else
    xml_assert(0, "not supprt xmlParseMemory in internal xml!!!!");
    return NULL;
#endif
}

void HW1xmlDocDumpMemory(hw_xmlDocPtr cur, hw_xmlChar **mem, int *size) 
{
#ifdef USE_libxm2lib
    xmlDocDumpMemory(cur, mem, size);
    return;
#else
    xml_assert(0, "\t not supprt xmlDocDumpMemory in internal xml!!!!\n");
    return ;
#endif
}



void HW1xmlFreeDoc(hw_xmlDocPtr cur) 
{
#ifdef USE_libxm2lib
    return xmlFreeDoc(cur);
#else
    return hw_xmlFreeDoc(cur);
#endif
}

void HW1xmlCleanupParser(void) 
{
#ifdef USE_libxm2lib
    return xmlCleanupParser();
#else
    return hw_xmlCleanupParser();
#endif
}

void *Hw1Memset(void *s, int ch, size_t n)
{
    return memset(s, ch, n);
}

#ifdef __cplusplus
}
#endif

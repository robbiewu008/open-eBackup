/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180
Creat at 2020-01-19

*/
#ifndef __XML_H__
#define __XML_H__


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>	 // sqrt cos log

// 不要与算法库产生太多纠缠，以便以后替换变异算法
#include "../../Secodefuzz/secodeFuzz.h"


#ifdef USE_libxm2lib
#include "../../examples/xml-lib/libxml2-2.6.26/include/libxml/tree.h"
#include "../../examples/xml-lib/libxml2-2.6.26/include/libxml/parser.h" 

#define hw_xmlDocPtr xmlDocPtr
#define hw_xmlNodePtr xmlNodePtr
#define hw_xmlChar xmlChar

#else
#include "../ex_lib/xml/Etree.h"
#include "../ex_lib/xml/Eparser.h" 
#endif


#ifdef __linux__
#include <arpa/inet.h>
#endif

#include <unistd.h>

#ifdef _WIN32
#include<ws2tcpip.h>

#include <windows.h>
#pragma comment(lib, "wsock32.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef Open_source
#define STRUCT_STRING                   "Struct"
#define UNION_STRING                     "Union"
#define INTEGER_STRING                 "Integer"
#define SECODEFUZZ_STRING          "Secodefuzz"
#define BUF_STRING                         "Buf"
#define TLV_STRING                          "Tlv"
#define CONVERSION_STRING           "Conversion"
#define HASH_STRING                       "Hash"
#define BITWIDTH_STRING               "bitWidth"
#define CHECK_STRING                     "check"
#else
#define STRUCT_STRING                   "Block"
#define UNION_STRING                    "Choice"
#define INTEGER_STRING                "Number"
#define SECODEFUZZ_STRING          "Peach"
#define BUF_STRING                        "Blob"
#define TLV_STRING                         "Relation"
#define CONVERSION_STRING           "Transform"
#define HASH_STRING                       "Fixup"
#define BITWIDTH_STRING               "size"
#define CHECK_STRING                     "token"
#endif

#define MAX_PITS_PARA_NUMBER  10                   // pits里，最大的param数量
#define MAX_XPATH_NAME_LENGTH  512
#define DISPLAY_MAX_BIN_LENGTH  2600             // debug时，打印最大的bin长度
#define DISPLAY_MAX_VALUE_LENGTH  256           // debug时，打印最大的value长度

#define MAX_SELF_COUNT 50  // 支持最大的fixup数量
#define MAX_DATAMODEL_COUNT 10  // 支持最大的fixup数量

enum Enum_Return {
    R_No	= 0,
    R_Yes	= 1,
    R_Complete = 2,
};

enum Enum_Action_Type {
    ENUM_ACTION_OUTPUT 		= 0,		//
    ENUM_ACTION_INPUT,				//
    ENUM_ACTION_OPEN,				//
    ENUM_ACTION_CLOSE,				//
    ENUM_ACTION_MAX=20,
};

typedef struct  XMLElement{
    char * 	        name;
    char * 	        typeName;
    int	 	        type;					
    int 		        length;
    int 		        xmlValueLength;
    int 		        size;

    int 		        position;

    u8 * 	               value;
    s64 		        numberValue;	// 64位好办事
    u8 * 	               stringValue;
    u8 * 	               blobValue;
    u8 * 	               paddingValue;

    int 		        isnullTerminated;

    // relation
    int	               isRelation;
    char *                relationType;
    char *                RelationOf;

    char *                expressionGet;
    char *                expressionSet;

    // constraint
    char *                constraint;

    // fixup
    int	                isFixup;
    char *                 className;
    char *                 paramName[MAX_PITS_PARA_NUMBER];
    char *                 paramValue[MAX_PITS_PARA_NUMBER];

    // Transformer
    int	                isTransformer;
    char *                 className1;
    char *                 paramName1[MAX_PITS_PARA_NUMBER];
    char *                 paramValue1[MAX_PITS_PARA_NUMBER];

    // Analyzer
    int	                isAnalyzer;
    char *                 analyzerClassName;
    int                       analyzerIsInit;
    char *                 analyzerValue1;
    char *                 analyzerValue2;

    // padding
    char *                 alignment;
    char *                 alignedTo;
    
    int                       isHasRef;
    hw_xmlNodePtr    xmlRef;

    int                       isToken;
    int                       isMutable;
    int                       isSigned;

    // String
    int                       isStringNumber;

    int                       isOccurs;
    u32                     occurs;
    u32                     minOccurs;
    u32                     maxOccurs;

    // StateModel
    char *                  initialState;

    // action
    char*                    actionType;
    char*                    publisher;
    char*                    method;
    char*                    dataModelRef;

    char*                    fileName;
    char*                    isclose;

    // test
    int 			     maxOutputSize;
    char* 			     stateModelRef;
    char* 			     publisherName;
    char* 			     publisherClass;
    char * 		     publisherParamName[MAX_PITS_PARA_NUMBER];
    char * 		     publisherParamValue[MAX_PITS_PARA_NUMBER];
    hw_xmlNodePtr       xmlNode;

    // inclue
    char* 	                   includeNs;
    char* 	                   includeSrc;
    hw_xmlNodePtr       includeRootDoc;
}SXMLElement;


typedef struct  MutatorElement{
    char  	                  xpathName[MAX_XPATH_NAME_LENGTH];
    SXMLElement *      xmlElement;

    int                         isRelationParentAndChild;

    int                         relationOfIs;
    char *                    relationOfRelationName;

    char *                    lastRelationOfS;
    char *                    lastRelationS;
    int                          lastIsRelationS;

    char *                    lastRelationOfC;
    char *                    lastRelationC;
    int                          lastIsRelationC;


    struct MutatorElement *next;	        /* next sibling link  */
    struct MutatorElement *prev;	        /* previous sibling link  */
    struct MutatorElement *children;	 /* next sibling link  */
    struct MutatorElement *parent;	 /* previous sibling link  */
    struct MutatorElement *seletedChildren; /* for choice  */
    struct MutatorElement *lastChildren;	/* next sibling link  */

}SMutatorElement;


typedef struct  BinElement{
    char  	                   xpathName[MAX_XPATH_NAME_LENGTH];
    SMutatorElement *  mutatorElement;


    //这里边的值都是不会释放的
    char * 	            defaultValue;
    s64 		            defaultNumberValue;
    char * 	            defaultStringValue;
    char * 	            defaultBlobValue;
    char * 	            defaultPaddingValue;

    int 		            defaultLength;
    int 		            defaultLengthHasSolve; // 有可能relation指明的长度就是0
    int 		            defaultCount;

    char* 	                   binOffset;

    char * 	            mutaterValue;
    int 		            mutaterLength;
    int 	 	            mutaterBitCount;
    s64 		            mutaterValueNumber;
    int                          hasMutator;

    s64 		            tempNumberValueMemory;
    char  	                   solvedXpathName[MAX_XPATH_NAME_LENGTH];

    struct BinElement *next; 	/* next sibling link  */
    struct BinElement *prev;	/* previous sibling link  */
    struct BinElement *children;	/* next sibling link  */
    struct BinElement *parent;	/* previous sibling link  */
    struct BinElement *seletedChildren; /* for choice  */
    struct BinElement *lastChildren;	/* next sibling link  */

}SBinElement;

typedef struct TestElement _STestElement;

// state模型结构体
typedef struct  StateElement{
    char  			        xpathName[MAX_XPATH_NAME_LENGTH];
    int 				 actionType;
    SXMLElement * 	        xmlElement;

    char * 			 refDataModeName;
    SBinElement * 	        binElement;
    SMutatorElement *     mutatorElement;

    _STestElement *        testElement;

    hw_xmlNodePtr 		 dataModeRoot;
    int  				 isParse;

    char * 			 outBuf;
    int  				 outLength ;

    char* 			        dataModelRef;
    char* 			        binFileName;
    char * 			 binBuf;
    int 				 binLength;
    char* 			        isClose;

    char* 			        publihserName;


    struct StateElement *      next;	/* next sibling link  */
    struct StateElement *      prev;	/* previous sibling link  */
    struct StateElement *      children;	/* next sibling link  */
    struct StateElement *      parent;	/* previous sibling link  */
    struct StateElement *      seletedChildren; /* for choice  */
    struct StateElement *      lastChildren;	/* next sibling link  */
}SStateElement;


// test模型结构体
typedef struct  TestElement{
    char  			        xpathName[MAX_XPATH_NAME_LENGTH];

    hw_xmlNodePtr 		 dataModeRoot;
    SXMLElement * 	        xmlElement;

    char* 			        stateModelRef;
    SStateElement*         stateElement;

    int                             publisherCount;
    char*			        publisherName[MAX_PITS_PARA_NUMBER];
    char*			        publisherClass[MAX_PITS_PARA_NUMBER];
    char*			        publisherParamName[MAX_PITS_PARA_NUMBER][MAX_PITS_PARA_NUMBER];
    char*			        publisherParamValue[MAX_PITS_PARA_NUMBER][MAX_PITS_PARA_NUMBER];
}STestElement;

typedef struct  {
    int mutatorBufCut;
    int mutatorBlockDelete;
    int mutatorBlockCopy;
    int mutatorBlockChange;
    int mutatorElementDelete;
    int mutatorElementCopy;
    int mutatorElementChange;
    int mutatorRelationsize;
}SPitsMutator;

struct PublisherGroup {
    char*                name;
    int 			no;
    int 			(*open)(char* name, SStateElement* tempAction);
    int 			(*output)(char* name, char* buf, int length, SStateElement* tempAction);
    int 			(*close)(char* name, SStateElement* tempAction);
};

struct FixupGroup {
    char*                name;
    int 			no;
    void 			(*fixup)(SBinElement* temp);
};

struct TransformerGroup {
    char*                name;
    int 			no;
    void 			(*transformer)(SBinElement* temp);
};


extern char * g_binName;
extern char * g_binBuf;
extern int g_binBufLength;
extern SPitsMutator g_pitsMutator;
extern int g_mutatorFrequency;
extern int g_algorithmFrequency;

extern int g_isSwap ;

extern int g_configNum;
extern char *g_configValue[];
extern char *g_configKey[];
extern int g_tempElementCount;;

extern char g_outBuf1[OUT_BUF_MAX_LENGTH];
extern char g_outBuf2[OUT_BUF_MAX_LENGTH];

extern int g_publisherCount;
extern  struct PublisherGroup* g_publisherGroup[MAX_SELF_COUNT];

extern int g_fixupCount;
extern  struct FixupGroup* g_fixupGroup[MAX_SELF_COUNT];

extern int g_transformerCount;
extern  struct TransformerGroup* g_transformerGroup[MAX_SELF_COUNT];

extern char* g_publisherBuf;
extern int g_publisherBufLen;

extern int g_onOffDebugMutatorElement;
extern int g_onOffDebugParseAssociated;
extern int g_onOffDebugGetBinBuf;
extern int g_onOffDebugParseBin;
extern int g_onOffDebugParseDataModel;
extern int g_onOffDebugParseStateModel;
extern int g_onOffDebugParseTestModel;
extern int g_onOffDebugParseXml;
extern int g_onOffDebugPublisher;
extern int g_onOffDebugDoRelation;
extern int g_onOffDebugDoFixup;
extern int g_onOffDebugDoTransformer;
extern int g_onOffDebugMutatorPits;



//Xcommon.h
extern hw_xmlDocPtr g_doc[];
extern int g_docNum;
extern char *g_testcaseMemory[];
extern int g_testcaseMemoryCount;
extern char *g_onerunMemory[];
extern int g_onerunMemoryCount;
extern int g_maxOutOfMutator;



#define For_Tree_Start(temp)  \
	while(temp != NULL)\
 	{\
 		{


#define For_Tree_End(temp)  \
		}\
		if(temp->children != NULL)\
		{\
			temp = temp->children;\
		}\
		else if(temp->next != NULL)\
		{\
			temp =temp->next;\
		}\
		else\
		{\
			temp=temp->parent;\
			if(temp == NULL)\
				break;\
			while(temp->next ==NULL)\
			{\
				temp =temp->parent;\
				if(temp == NULL)\
					break;\
			}\
			if(temp == NULL)\
				break;\
			temp=temp->next;\
		}\
	}


#define For_Tree_child_Start(temp)  \
	while(temp != NULL)\
 	{\
 		{


#define For_Tree_child_End(temp,parent)  \
		}\
		if(temp->children != NULL)\
		{\
			temp = temp->children;\
		}\
		else if(temp->next != NULL)\
		{\
			temp =temp->next;\
		}\
		else\
		{\
			temp=temp->parent;\
			if(temp == parent)\
				break;\
			while(temp->next ==NULL)\
			{\
				temp =temp->parent;\
				if(temp == parent)\
					break;\
			}\
			if(temp == parent)\
				break;\
			temp=temp->next;\
		}\
	}

#include <assert.h>


#define xml_assert(temp ,string)  \
			if(temp == 0)\
			{\
				printf("*****error --%s-- on file %s  line %d\n",string, __FILE__ ,__LINE__ ); \
				assert(0);\
			}


// secodefuzz.so
extern void HexDump(u8 *buf, u32 len);
extern u32 GaussRandU32(u32 pos);
extern char *Inltoa(s64 value, char *string, int radix);
extern int HwRand();
extern void ReadFromFile(char** data, int *len, char *path);

int ParseXmlHW(char* docname, char* dataModelName, int isfirst);
void DoActionHW(int id);

/**********************************************************
ParseXml 系列
**********************************************************/
// XParseXml.c
hw_xmlNodePtr ParseXml(char* docname, int needparseconfig);
// XParseXml_Config.c
void ParseXmlConfig(char* docname);
// XParseXml_TestModel.c
void ParseXmlTestModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent);
// XParseXml_StateModel.c
void ParseXmlStateModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent);
// XParseXml_DataModel.c
void ParseXmlDataModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent);

/**********************************************************
Parse 系列
**********************************************************/
// XParseTestModel.c
STestElement* ParseTestModel(hw_xmlNodePtr root, char* testModelName);

// XParseStateModel.c
SStateElement* ParseStateModel(hw_xmlNodePtr root, char* stateModelName, STestElement* tempTestElement);

// XParseDataModel.c
SMutatorElement* ParseDataModel(hw_xmlNodePtr root, char* dataModelName);

// XParseAssociated.c
void ParseAssociated(SMutatorElement* tempMutatorElement);

// XParseBin.c
SBinElement* ParseBin(SMutatorElement* tempMutatorElement, char* binBuf, int length);

/**********************************************************
Do 系列
**********************************************************/
// XDoAction.c
void DoActionOpen(SStateElement* tempAction);
void DoActionClose(SStateElement* tempAction);
void DoActionOutput(SStateElement* tempAction);
void DoActionInput(SStateElement* tempAction);

// XDoState.c
void DoState(SStateElement* tempAction, STestElement* tempTestElement);

// XDoPadding.c
void DoPadding(SBinElement* temp);

// XDoRelation.c
void DoRelation(SBinElement* temp);
int DoExpressionGet(int size, char* str);
int DoExpressionSet(int size, char* str);


// XDoFixup.c
void DoFixup(SBinElement* temp);
int RegisterFixupGroup( struct FixupGroup* fixupGroup);
void DoFixupInit(void);

// XDoAnalyzer.c
void DoAnalyzer(SBinElement* temp);
void AnalyzerClean(void);

// XDoTransformer.c
void DoTransformer(SBinElement* temp);
int RegisterTransformerGroup( struct TransformerGroup* transformerGroup);
void DoTransformerInit(void);


// XDoconstraint.c
int DoConstraint(SXMLElement *temp_xml);

// XDoPublisher
extern void DoPublisherInit(void);
extern void DoPublisherOpen(char* name, SStateElement* tempAction);
extern void DoPublisherOutput(char* name, char* buf, int length, SStateElement* tempAction);
extern void DoPublisherClose(char* name, SStateElement* tempAction);

int GetPublisherId_ByName(char* name, SStateElement* tempAction);
char* GetPublisherParaValue_ByName(int id, char* name, SStateElement* tempAction);
extern int RegisterPublisherGroup( struct PublisherGroup* publisherGroup);

/**********************************************************
变异 系列
**********************************************************/
// XMutatorElement.c
void GetMutatorElementValue(SBinElement* temp);

// XMutatorPits.c
int GetPitsBuf(SBinElement* temp, char* buf);
int GetPitsBufNoMutator(SBinElement* temp, char* buf);
int GetPitsBufLength(SBinElement * parent);

/**********************************************************
其他 系列
**********************************************************/
// XFileIO.h
 int GetBinBuf(char* fileName, char** buf, int *length);

// debug
void DoDebugOpen(char* debug);

// XExternal.c
extern hw_xmlDocPtr HW1xmlReadFile(const char *filename, const char *encoding, int options);
extern hw_xmlNodePtr HW1xmlDocGetRootElement(hw_xmlDocPtr doc);
extern hw_xmlChar* HW1xmlGetProp(hw_xmlNodePtr node, const hw_xmlChar *name);
extern hw_xmlDocPtr HW1xmlParseMemory(const char *buffer, int size);
extern void HW1xmlDocDumpMemory(hw_xmlDocPtr cur, hw_xmlChar **mem, int *size);

extern void HW1xmlFreeDoc(hw_xmlDocPtr cur); 
extern void HW1xmlCleanupParser(void); 
void *Hw1Memset(void *s, int ch, size_t n);


// Xlib
s64 Hex2Dec(u8 *hex);
void HexStrToByte(const char* source, unsigned char* dest, int sourceLen);

u16 BigLittleSwap16(u16 A, int is_swap);     
u32 BigLittleSwap32(u32 A, int is_swap);    
u64 BigLittleSwap64(u64 A, int is_swap);

int TrimSpace(u8 *inbuf, u8 *outbuf);
u32 FromIpstrToUint(char* ip );
char* MyMemmem(char* a, int alen, char* b, int blen);
unsigned short CheckSum(unsigned short *buf, int nword);
unsigned int CheckSum32( const unsigned char *buf, unsigned int size);
int GetLengthFromSize(int size);
char* GetConfigValueByKey(char* key) ;
char* strip_config(char* value);

/**********************************************************
bit操作函数
**********************************************************/
extern s64 GetBufBitValue(int position, u8* buf);
extern void SetBufBitValue(int position, u8* buf, u8 value);
extern s64 GetNumberBinValue(int position, int size, int tsigned, u8* buf);
extern void SetNumberBinValue(int position, int size, int tsigned, u8* buf, s64 bbbb);
extern s64 GetSomeBitValue(s64 temp, int posion, int size, int allsize);
extern void SetSomeBitValue(s64 *value, s64 temp, int posion, int size, int allsize);

/**********************************************************
tree操作函数
**********************************************************/
// xml 
extern char* XmlGetProperty(hw_xmlNodePtr node, char * property_name);
extern char* XmlGetGetProp(hw_xmlNodePtr node, char *name);
extern hw_xmlNodePtr XmlGetDataModelByName(hw_xmlNodePtr par, char* name);
extern hw_xmlNodePtr XmlGetStateModelNodeByName(hw_xmlNodePtr par, char* name);
extern hw_xmlNodePtr XmlGetStateModelDocByName(hw_xmlNodePtr par, char* name);
extern hw_xmlNodePtr XmlGetTestModelByName(hw_xmlNodePtr par, char* name);

// mutator
extern int MutatorElementIsSameElement(SMutatorElement* temp,char *name);
extern SMutatorElement* MutatorElementFoundRelationofByName(SMutatorElement* temp1, char *name);
extern SMutatorElement* MutatorElementFoundByXpathName(char* name, SMutatorElement* parent);
extern SMutatorElement* MutatorElementFoundLast(SMutatorElement* temp);
extern int MutatorElementIsParent(SMutatorElement* temp, SMutatorElement* ppp);
extern void MutatorElementAddChildren(SMutatorElement* temp_parent, SMutatorElement* temp);

// bin
extern int BinElementIsSameElement(SBinElement* temp,char *name);
extern SBinElement* BinElementFoundRelationofByName(SBinElement* temp1, char *name);
extern SBinElement* BinElementFoundRecentlyByXpathName(SBinElement* temp, char *xpathName);
extern void BinElementAddChildren(SBinElement* tempParent, SBinElement* temp);
extern void BinElementDelChildren(SBinElement* tempParent, SBinElement* temp);
extern int  BinElementGetOffsetLength(SBinElement* parent, SBinElement* aaaa);

#ifdef __cplusplus
}
#endif
#endif

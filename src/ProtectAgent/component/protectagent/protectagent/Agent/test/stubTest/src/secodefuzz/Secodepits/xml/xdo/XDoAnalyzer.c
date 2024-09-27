/*
版权所有 (c) 华为技术有限公司 2012-2018


分发fixup的函数

<DataModel name="crc32_datamodel" >
	<Blob name="Data" value="huawei is ok!" />
	<Number name="CRC" size="32" >
		<Fixup class="crc32">
			<Param name="ref" value="Data" />
		</Fixup>
	</Number>
</DataModel>

*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

void DoJsonAnalyzer(SBinElement* temp);
void JsonAnalyzerClean(void);

void DoXmlAnalyzer(SBinElement* temp);
void XmlAnalyzerClean(void);


void DoAnalyzer(SBinElement* temp)
{
    SXMLElement* tempXml = temp->mutatorElement->xmlElement;
    if (strcmp(tempXml->analyzerClassName, "Json") == 0)
    {
        DoJsonAnalyzer(temp);
    }

    if (strcmp(tempXml->analyzerClassName, "Xml") == 0)
    {
        DoXmlAnalyzer(temp);
    }
} 

void AnalyzerClean(void)
{
    JsonAnalyzerClean();

    XmlAnalyzerClean();
} 

#ifdef __cplusplus
}
#endif

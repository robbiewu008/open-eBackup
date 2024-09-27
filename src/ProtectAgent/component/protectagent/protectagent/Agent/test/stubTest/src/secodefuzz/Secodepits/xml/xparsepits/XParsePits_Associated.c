/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


����ָ������ģ�ͣ�������Ҫ�����Ԫ�ض������������ref����

������choice�������������
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

// �����ڽ��ref֮������������飬�����޷�֪���Ǹ�Ԫ��������Ԫ��********************
// relation fixup tranform���ֶ���ϵ����Ҫ��������
void ParseAssociated(SMutatorElement* tempMutatorElement)
{
    if (g_onOffDebugParseAssociated)
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n");   
        printf("****Parse Associated Start\r\n");  
        printf("* * * * * * * * * * * * * *\r\n");  
    }

    SMutatorElement* temp = tempMutatorElement;
    For_Tree_Start(temp)
    {
        // ��������xml��������relation��Ԫ��
        if (temp->xmlElement->isRelation)
        {
            if (strcmp(temp->xmlElement->relationType, "size") == 0)
            {
                if (g_onOffDebugParseAssociated)
                {
                    printf("relation size:  <Relation>=%s \r\n", temp->xpathName);
                }

                if (g_onOffDebugParseAssociated)
                {
                    printf("                <of>      =%s \r\n", temp->xmlElement->RelationOf);
                }

                SMutatorElement* relation_of = MutatorElementFoundRelationofByName(temp, temp->xmlElement->RelationOf);

                // û���ҵ��ǲ�����ˡ�ģ������޷�������ȥ��
                if (relation_of == NULL)
                {            
                    xml_assert(0, "Parse Associated error");
                }


                // �ж�relation�����Ƿ��Ǹ��ӻ���ү����洫��ϵ
                relation_of->isRelationParentAndChild = MutatorElementIsParent(temp, relation_of);

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                <of_xpath>=%s \r\n", relation_of->xpathName);
                }

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                is parent and child   %d\r\n", relation_of->isRelationParentAndChild);
                }

                relation_of->relationOfIs =1;
                relation_of->relationOfRelationName = temp->xpathName;
                // relation_of->last_is_relation_s = 1;

                // �ҵ����һ��Ԫ��
                SMutatorElement* last = MutatorElementFoundLast(relation_of);

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                <last>    =%s \r\n\r\n", last->xpathName);
                }

                last->lastRelationOfS = relation_of->xpathName;
                last->lastRelationS = temp->xpathName;
                last->lastIsRelationS = 1;
            }

            if (strcmp(temp->xmlElement->relationType ,"count") == 0)
            {
                if (g_onOffDebugParseAssociated)
                {            
                    printf("relation count: <Relation>=%s \r\n", temp->xpathName);
                }

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                <of>      =%s \r\n", temp->xmlElement->RelationOf);
                }

                // �Ȳ����Լ��ĸ��ף�Ȼ�����ֵܣ�үү�����׵��ֵ�.... ....
                // �����ϲ������Լ�ǰ�ߵ�Ԫ�أ������߼����ң�������ӣ�xml��д��ʱ�������
                SMutatorElement* relation_of = MutatorElementFoundRelationofByName(temp, temp->xmlElement->RelationOf);

                if (relation_of == NULL)
                {            
                    xml_assert(0, "Parse Associated error");
                }

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                <of_xpath>=%s \r\n", relation_of->xpathName);
                }

                relation_of->relationOfIs = 1;

                // ����count��˵��relation of����last
                SMutatorElement* last = relation_of;

                if (g_onOffDebugParseAssociated)
                {            
                    printf("                <last>    =%s \r\n\r\n", last->xpathName);
                }

                last->lastRelationOfC = relation_of->xpathName;
                last->lastRelationC = temp->xpathName;
                last->lastIsRelationC = 1;
            }
        }

        // ��������xml��������fixup��Ԫ��
        if (temp->xmlElement->isFixup)
        {
            if (g_onOffDebugParseAssociated)
            {         
                printf("fixup:  xpath_name  =%s \r\n", temp->xpathName);
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <class>    =%s \r\n", temp->xmlElement->className);
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <ref>      =%s \r\n", temp->xmlElement->paramValue[0]);
            }

            SMutatorElement* ref = MutatorElementFoundRelationofByName(temp, temp->xmlElement->paramValue[0]);

            // û���ҵ��ǲ�����ˡ�ģ������޷�������ȥ��
            if (ref == NULL)
            {         
                xml_assert(0, "Parse Associated error");
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <ref_xpath>=%s \r\n", ref->xpathName);
            }
        }

        // ��������xml��������transformer��Ԫ��
        if (temp->xmlElement->isTransformer)
        {
            if(g_onOffDebugParseAssociated)
            {         
                printf("transformer:  xpath_name  =%s \r\n", temp->xpathName);
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <class>    =%s \r\n", temp->xmlElement->className1);
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <ref>      =%s \r\n", temp->xmlElement->paramValue1[0]);
            }

            SMutatorElement* ref = MutatorElementFoundRelationofByName(temp, temp->xmlElement->paramValue1[0]);

            // û���ҵ��ǲ�����ˡ�ģ������޷�������ȥ��
            if (ref == NULL)
            {         
                xml_assert(0, "Parse Associated error");
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <ref_xpath>=%s \r\n", ref->xpathName);
            }
        }
    }
    For_Tree_End(temp)

    if (g_onOffDebugParseAssociated)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("****Parse Associated  End\r\n");  
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n");  
    }
}

#ifdef __cplusplus
}
#endif

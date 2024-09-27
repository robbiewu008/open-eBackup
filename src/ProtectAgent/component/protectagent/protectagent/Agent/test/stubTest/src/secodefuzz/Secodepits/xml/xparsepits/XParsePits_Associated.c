/*
版权所有 (c) 华为技术有限公司 2012-2018


解析指定数据模型，生成需要变异的元素二叉树，并解决ref问题

数组与choice仅解析，不解决
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

// 必须在解决ref之后来搞这个事情，否则无法知道那个元素是最后的元素********************
// relation fixup tranform等字段联系，需要在这里解决
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
        // 遍历所有xml声明里有relation的元素
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

                // 没有找到是不可饶恕的，程序无法进行下去了
                if (relation_of == NULL)
                {            
                    xml_assert(0, "Parse Associated error");
                }


                // 判断relation两者是否是父子或者爷孙等祖传关系
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

                // 找到最后一个元素
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

                // 先查找自己的父亲，然后是兄弟，爷爷，父亲的兄弟.... ....
                // 理论上不能找自己前边的元素，否则逻辑混乱，这个复杂，xml编写的时候请避免
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

                // 对于count来说，relation of就是last
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

        // 遍历所有xml声明里有fixup的元素
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

            // 没有找到是不可饶恕的，程序无法进行下去了
            if (ref == NULL)
            {         
                xml_assert(0, "Parse Associated error");
            }

            if (g_onOffDebugParseAssociated)
            {         
                printf("                <ref_xpath>=%s \r\n", ref->xpathName);
            }
        }

        // 遍历所有xml声明里有transformer的元素
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

            // 没有找到是不可饶恕的，程序无法进行下去了
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

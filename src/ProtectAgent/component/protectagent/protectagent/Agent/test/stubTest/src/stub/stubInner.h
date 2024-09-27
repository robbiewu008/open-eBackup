/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef __DYNAMIC_STUB__
#define __DYNAMIC_STUB__

#define EXTERNC extern "C"

#include <stdlib.h>
#include <stdio.h>
const int ADDR_TEXT_LEN = 20;

/*******************************************************************************
  ������          : init
  ��������        : ��ʼ�����������Գ���ʼʱ����һ�Σ�������cppunit���������е��ã�
                    ��Windowsƽ̨�£����Բ����ô˺���
  �������        : ��
  �������        : ��
  ����ֵ          : 0 ��ʾ�ɹ���������ʾʧ��  
******************************************************************************/
EXTERNC  int stubInit();

/*******************************************************************************
  ������          : _setStub
  ��������        : ��̬��׮��������Ҫֱ��ʹ�����������ʹ�������setStub����
  �������        :    pOldFunc ԭʼ����
                  :    pNewFunc �����滻ԭʼ�����ĺ���
  �������        : �� 
  ����ֵ          : ��������׮�������ţ������ʱʹ�ã���������ʾʧ��
******************************************************************************/
EXTERNC  int _setStub(void *pOldFunc, void *pNewFunc, void *pObj);

/*******************************************************************************
  ������          : setStubC
  ��������        : ��ȫ�ֺ����滻ȫ�ֺ�����C�������������
  �������        :    pOldFunc ԭʼ����
                  :    pNewFunc �����滻ԭʼ�����ĺ���
  �������        : �� 
  ����ֵ          : ��������׮�������ţ������ʱʹ�ã���������ʾʧ��
******************************************************************************/
EXTERNC int setStubC(void *pOldFunc, void *pNewFunc);

#ifdef __cplusplus

/*******************************************************************************
  ������          : getMemberAddr
  ��������        : ��ȡC++�����ĵ�ַ
  �������        : �����������磺f1, &A::f1��
  �������        : ��
  ����ֵ          : ������ַ
******************************************************************************/
template<typename T>
void *getMemberAddr(T pf)
{
    char pBuf[ADDR_TEXT_LEN];
    sprintf(pBuf, "%ld,", pf);
    return (void*)atol(pBuf);
}
/*******************************************************************************
  ������          : setStub
  ��������        : ��һ��ȫ�ֺ�����һ�����麯����׮�����麯��������ȫ�ֺ�����
                      ��ľ�̬�����������ͨ��Ա����
  �������        : pOldFunc ԭʼ����
                  : pNewFunc �����滻ԭʼ������׮����
  �������        : ��
  ����ֵ          : ��������׮�������ţ������ʱʹ�ã���������ʾʧ��
******************************************************************************/
template<typename FunctionPointerTypeOld, typename FunctionPointerTypeNew>
int setStub(FunctionPointerTypeOld pOldFunc,       
            FunctionPointerTypeNew pNewFunc)       
{
    return _setStub(getMemberAddr(pOldFunc), getMemberAddr(pNewFunc), NULL);
}

/*******************************************************************************
  ������          : setStub
  ��������        : ��һ��ȫ�ֺ�����һ���麯����׮
  �������        : pOldFunc ԭʼ����
                  : pNewFunc �����滻ԭʼ������׮����
                    pObject �麯����Ӧ���һ�������ָ��
  �������        : ��
  ����ֵ          : ��������׮�������ţ������ʱʹ�ã���������ʾʧ��
******************************************************************************/
template<typename FunctionPointerTypeOld, typename FunctionPointerTypeNew, typename ObjectType>
int setStub(FunctionPointerTypeOld pOldFunc,       //�������: ԭʼ����
            FunctionPointerTypeNew pNewFunc,       //�������: �滻ԭʼ�������º���
            ObjectType *pObject)       
{
    return _setStub(getMemberAddr(pOldFunc), getMemberAddr(pNewFunc), (void*)pObject);
}

#endif //#ifdef __cplusplus

/*******************************************************************************
  ������          : clearStub
  ��������        : �����������̬׮
  �������        : int iIdx����̬׮�����������������������̬׮
  �������        : ��
  ����ֵ          : 0��ʾ�ɹ���������ʾʧ��
******************************************************************************/
EXTERNC int clearStub(int iIdx);

/*******************************************************************************
  ������          : clearAllStub
  ��������        : ���֮ǰ���õ����ж�̬׮
  �������        : ��
  �������        : ��
  ����ֵ          : ��������̬׮�ĸ�����������ʾʧ��
********************************************************************************/
EXTERNC int clearAllStub();

/*******************************************************************************
  ������          : final
  ��������        : ���������������Գ��������ʱ�����
  �������        : ��
  �������        : ��
  ����ֵ          : void
********************************************************************************/
EXTERNC void stubFinal();

#endif //#ifndef __DYNAMIC_STUB__


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
  函数名          : init
  功能描述        : 初始化，整个测试程序开始时调用一次，比如在cppunit的主函数中调用，
                    在Windows平台下，可以不调用此函数
  输入参数        : 无
  输出参数        : 无
  返回值          : 0 表示成功，其它表示失败  
******************************************************************************/
EXTERNC  int stubInit();

/*******************************************************************************
  函数名          : _setStub
  功能描述        : 动态打桩函数，不要直接使用这个函数，使用下面的setStub函数
  输入参数        :    pOldFunc 原始函数
                  :    pNewFunc 用于替换原始函数的函数
  输出参数        : 无 
  返回值          : 返回所打桩的索引号（供清除时使用），负数表示失败
******************************************************************************/
EXTERNC  int _setStub(void *pOldFunc, void *pNewFunc, void *pObj);

/*******************************************************************************
  函数名          : setStubC
  功能描述        : 用全局函数替换全局函数，C语言用这个函数
  输入参数        :    pOldFunc 原始函数
                  :    pNewFunc 用于替换原始函数的函数
  输出参数        : 无 
  返回值          : 返回所打桩的索引号（供清除时使用），负数表示失败
******************************************************************************/
EXTERNC int setStubC(void *pOldFunc, void *pNewFunc);

#ifdef __cplusplus

/*******************************************************************************
  函数名          : getMemberAddr
  功能描述        : 获取C++函数的地址
  输入参数        : 函数名，形如：f1, &A::f1等
  输出参数        : 无
  返回值          : 函数地址
******************************************************************************/
template<typename T>
void *getMemberAddr(T pf)
{
    char pBuf[ADDR_TEXT_LEN];
    sprintf(pBuf, "%ld,", pf);
    return (void*)atol(pBuf);
}
/*******************************************************************************
  函数名          : setStub
  功能描述        : 用一个全局函数给一个非虚函数打桩，非虚函数包括：全局函数、
                      类的静态函数、类的普通成员函数
  输入参数        : pOldFunc 原始函数
                  : pNewFunc 用于替换原始函数的桩函数
  输出参数        : 无
  返回值          : 返回所打桩的索引号（供清除时使用），负数表示失败
******************************************************************************/
template<typename FunctionPointerTypeOld, typename FunctionPointerTypeNew>
int setStub(FunctionPointerTypeOld pOldFunc,       
            FunctionPointerTypeNew pNewFunc)       
{
    return _setStub(getMemberAddr(pOldFunc), getMemberAddr(pNewFunc), NULL);
}

/*******************************************************************************
  函数名          : setStub
  功能描述        : 用一个全局函数给一个虚函数打桩
  输入参数        : pOldFunc 原始函数
                  : pNewFunc 用于替换原始函数的桩函数
                    pObject 虚函数相应类的一个对象的指针
  输出参数        : 无
  返回值          : 返回所打桩的索引号（供清除时使用），负数表示失败
******************************************************************************/
template<typename FunctionPointerTypeOld, typename FunctionPointerTypeNew, typename ObjectType>
int setStub(FunctionPointerTypeOld pOldFunc,       //输入参数: 原始函数
            FunctionPointerTypeNew pNewFunc,       //输入参数: 替换原始函数的新函数
            ObjectType *pObject)       
{
    return _setStub(getMemberAddr(pOldFunc), getMemberAddr(pNewFunc), (void*)pObject);
}

#endif //#ifdef __cplusplus

/*******************************************************************************
  函数名          : clearStub
  功能描述        : 按索引清除动态桩
  输入参数        : int iIdx，动态桩索引，该索引可用于清除动态桩
  输出参数        : 无
  返回值          : 0表示成功，其它表示失败
******************************************************************************/
EXTERNC int clearStub(int iIdx);

/*******************************************************************************
  函数名          : clearAllStub
  功能描述        : 清除之前设置的所有动态桩
  输入参数        : 无
  输出参数        : 无
  返回值          : 返回清理动态桩的个数，负数表示失败
********************************************************************************/
EXTERNC int clearAllStub();

/*******************************************************************************
  函数名          : final
  功能描述        : 清理函数，整个测试程序结束的时候调用
  输入参数        : 无
  输出参数        : 无
  返回值          : void
********************************************************************************/
EXTERNC void stubFinal();

#endif //#ifndef __DYNAMIC_STUB__


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
#ifndef __STUB_H__
#define __STUB_H__

#include "stubInner.h"
#include <stdio.h>

template <typename FunctionPointerTypeOld, typename FunctionPointerTypeNew, typename ObjectType>
class Stub
{
private:
    int idx;
public:
    Stub(FunctionPointerTypeOld pOldFunc, FunctionPointerTypeNew pNewFunc, ObjectType* pobj)
    {
        idx = setStub(pOldFunc, pNewFunc, pobj);

        if(idx < 0)
        {
            printf("Fail to set stub C!!!\n");
            throw "Fail to set stub C!!!";
        }
    }
    Stub(FunctionPointerTypeOld pOldFunc, FunctionPointerTypeNew pNewFunc){
       idx = setStub(pOldFunc, pNewFunc);

        if(idx < 0)
        {
            printf("Fail to set stub C!!!\n");
            throw "Fail to set stub C!!!";
        }
    }
    virtual ~Stub()
    {
        clearStub(idx);
    }
    static void Init(){
      if(::stubInit() !=0 )
        {
            printf("Fail to init stub!!!\n");
            throw "Fail to init stub!!!\n";
        }
    }
    static void Destroy(){
      stubFinal();
    }
};

#endif

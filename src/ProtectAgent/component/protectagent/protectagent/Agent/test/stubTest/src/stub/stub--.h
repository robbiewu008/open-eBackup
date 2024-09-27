/******************************************************************************
  �ļ���          : stub.h
  �汾��          : 1.0
  ����            : ֣���
  ��������        : 2015-09-14
  �ļ�����        : ��̬��׮���߽ӿ������ļ���֧��SUSE10-x64ƽ̨
  ����            : 
                    ���ļ���װ��stubInner.h�еĺ�����C++���벻Ҫ
				      ʹ��stubInner.h��
					��stub���߲�����ɶԹ��캯�������������Ĵ�׮
  ����˵�� :
                    ʹ��Stub��ʵ�ֶ�̬��׮
					���������Կ�ʼ֮ǰ����Stub::Init();
					�������������֮�����Stub::Destroy();				
******************************************************************************/
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

/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file AfsObject.h
 * @brief AFS - A C/C++ Library for analyze format of a disk image.
 *
 */

#ifndef AFSOBJECT_H
#define AFSOBJECT_H

#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <string>
#include <map>
#include <iostream>
#include "afs/Afslibrary.h"

using namespace std;

#define OBJ_TYPE_UNKNOWN 0
#define OBJ_TYPE_IMGREADER 1
#define OBJ_TYPE_FILESYSTEM 2
#define OBJ_TYPE_PARTITION 3
#define OBJ_TYPE_LVM 4

class afsObject {
public:
    afsObject()
    {
        m_magic = "";
        m_initflag = false;
        m_objtype = OBJ_TYPE_UNKNOWN;
    }

    virtual ~afsObject() {};

    int getObjType()
    {
        return m_objtype;
    }

    void setObjType(int type)
    {
        m_objtype = type;
    }

    size_t getMagic(string &buf)
    {
        buf = m_magic;
        return m_magic.length();
    }

    void setMagic(string buf)
    {
        m_magic = buf;
    }

private:
    afsObject(const afsObject &obj);
    afsObject &operator = (const afsObject &obj);

    bool m_initflag;
    int m_objtype;
    string m_magic;
};

class afsObjectFacotry {
private:
    typedef afsObject *(*CreateAfsObject)(void);
    map<const string, CreateAfsObject> m_classMap;
    afsObjectFacotry *m_pInstance;
    afsObjectFacotry(const afsObjectFacotry &obj);
    afsObjectFacotry &operator = (const afsObjectFacotry &obj);

public:
    afsObjectFacotry()
    {
        m_pInstance = NULL;
    }

    virtual ~afsObjectFacotry()
    {
        if (m_pInstance != NULL) {
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

    afsObjectFacotry *getInstance()
    {
        if (NULL == m_pInstance) {
            m_pInstance = new afsObjectFacotry();
        }

        return static_cast<afsObjectFacotry *>(m_pInstance);
    }

    void registerClass(const string name, CreateAfsObject method)
    {
        m_classMap.insert(pair<const string, CreateAfsObject>(name, method));
    }

    void *createObjectByName(const string name)
    {
        map<const string, CreateAfsObject>::const_iterator iter;

        iter = m_classMap.find(name);
        if (iter == m_classMap.end()) {
            return NULL;
        }

        return iter->second();
    }
};

#endif

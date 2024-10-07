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
#ifndef CONFIGVALUE_H
#define CONFIGVALUE_H

#include <string>
#include <vector>
#include "common/IniFile.h"

namespace {
    const std::string MODULE_NAME = "ConfigValue";
}

namespace Module {

typedef enum 
{
    CONFIG_READER_ITEM_VALUE_INT,
    CONFIG_READER_ITEM_VALUE_STRING
} CONFIG_READER_ITEM_VALUE_TYPE;

class ConfigValueBase
{
public:
    virtual ~ConfigValueBase(){};

    //The process as below should be done while implementing this function:
    //Get value from config file, use default value version get of boost, ref:http://www.boost.org/doc/libs/1_41_0/doc/html/boost_propertytree/accessing.html
    //If the value is valid, set value to m_returnValue. Otherwize, set m_defaultValue to m_returnValue.
    //Log while the value is invalid.
    //Log new value.
    virtual void refreshValue(const std::string & sectionName, const std::string & keyName,
        std::vector<Module::CIniFile*> & iniFiles) = 0;

    virtual void setToDefaultValue() = 0;

    virtual void getValue(int & returnValue) const
    {
        returnValue = -1;
    }

    virtual void getValue(std::string & returnValue) const
    {
        returnValue = "";
    }

    virtual CONFIG_READER_ITEM_VALUE_TYPE getType() = 0;

    virtual int getMinValue() const
    {
        return -1;
    }

    virtual int getMaxValue() const
    {
        return -1;
    }

    //At least m_defaultValue and m_returnValue should be added as the member.
};

class IntConfigValue : public ConfigValueBase
{
public:
    IntConfigValue(int minValue, int maxValue, int defaultValue)
        :m_minValue(minValue), 
        m_maxValue(maxValue),
        m_defaultValue(defaultValue),
        m_returnValue(defaultValue)
    {}

    virtual ~IntConfigValue(){}; 

    virtual void refreshValue(const std::string & sectionName, const std::string & keyName, std::vector<Module::CIniFile*> & iniFiles);

    virtual void setToDefaultValue()
    {
        m_returnValue = m_defaultValue;
    }

    virtual void getValue(int & returnValue) const
    {
        returnValue = m_returnValue;
    }

    virtual CONFIG_READER_ITEM_VALUE_TYPE getType()
    {
        return CONFIG_READER_ITEM_VALUE_INT;
    }

    virtual int getMinValue() const
    {
        return m_minValue;
    }

    virtual int getMaxValue() const
    {
        return m_maxValue;
    }
       
private:
    int m_minValue;
    int m_maxValue;
    int m_defaultValue;
    int m_returnValue;
};

class StringConfigValue : public ConfigValueBase
{
public:
    StringConfigValue(const std::string & defaultValue)
        :m_defaultValue(defaultValue),
        m_returnValue(defaultValue)
    {}

    virtual ~StringConfigValue(){};

    virtual void refreshValue(const std::string & sectionName, const std::string & keyName,
        std::vector<Module::CIniFile*> & iniFiles);

    virtual void setToDefaultValue()
    {
        m_returnValue = m_defaultValue;
    }
 
    virtual void getValue(std::string & returnValue) const
    {
        returnValue = m_returnValue;
    }

    virtual CONFIG_READER_ITEM_VALUE_TYPE getType()
    {
        return CONFIG_READER_ITEM_VALUE_STRING;
    }
    
protected:
    std::string m_defaultValue;
    std::string m_returnValue;
};

class IPStringConfigValue : public StringConfigValue
{
public:
    IPStringConfigValue(const std::string & defaultValue)
        :StringConfigValue(defaultValue)
    {}
    virtual ~IPStringConfigValue() = default;
    
    virtual void refreshValue(const std::string & sectionName, const std::string & keyName,
        std::vector<Module::CIniFile*> & iniFiles);
};

} // namespace Module

#endif


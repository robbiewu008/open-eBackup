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
#ifndef ASSOCIATE_RESPONSE_H
#define ASSOCIATE_RESPONSE_H

#include <string>
#include <optional>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

struct ResourceRecord {
    std::vector<std::string> m_tags;        // 标签列表，可以为空，类型为字符串向量
    std::string m_resourceName;             // 资源名称，类型为字符串
    std::string m_resourceType;             // 资源类型，类型为字符串
    std::string m_resourceId;               // 资源ID，类型为字符串

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tags, tags);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceName, resourceName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceType, resourceType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceId, resourceId);
    END_SERIAL_MEMEBER;
};

struct TagsInfo {
    int32_t m_total;              // 总记录数，通常是整数
    int32_t m_size;               // 每页记录数，通常是整数
    int32_t m_current;            // 当前页码，通常是整数
    std::vector<std::string> m_orders; // 排序条件，假设为空表示没有排序条件
    bool m_optimizeCountSql;  // 是否优化计数SQL，布尔值
    bool m_hitCount;          // 是否启用缓存，布尔值
    std::string m_countId; // 计数ID，可能为空
    int32_t m_maxLimit; // 最大限制，可能为空
    bool m_searchCount;       // 是否查询总记录数，布尔值
    int32_t m_pages;              // 总页数，整数
    std::vector<ResourceRecord> m_records;
 
    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, total);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_current, current);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_orders, orders);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_optimizeCountSql, optimizeCountSql);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hitCount, hitCount);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_countId, countId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_maxLimit, maxLimit);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_searchCount, searchCount);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pages, pages);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_records, records);
    END_SERIAL_MEMEBER;
};
 
class AssociateResponse : public VirtPlugin::ResponseModel {
public:
    AssociateResponse() {}
    ~AssociateResponse() {}
 
    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_tagsInfo)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }
 
    TagsInfo GetTagsInfo()
    {
        return m_tagsInfo;
    }
private:
    TagsInfo m_tagsInfo;
};
};

#endif
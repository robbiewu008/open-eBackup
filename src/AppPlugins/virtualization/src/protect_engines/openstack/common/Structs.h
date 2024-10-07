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
#ifndef OPENSTACK_STRUCTS_H
#define OPENSTACK_STRUCTS_H
#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

struct Domain {
    std::string m_id;
    std::string m_name;
    std::string m_enabled;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    END_SERIAL_MEMEBER
};

struct DomainInfo {
    std::vector<Domain> m_domainlist;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainlist, domains)
    END_SERIAL_MEMEBER
};

struct ProjectInfo {
    std::string m_id;
    std::string m_name;
    std::string m_description;
    std::string m_domainId;
    bool m_enabled {false};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domain_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    END_SERIAL_MEMEBER
};

struct ProjectList {
    std::vector<ProjectInfo> m_projects;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projects, projects)
    END_SERIAL_MEMEBER
};

struct ServiceInfo {
    std::string m_id;
    std::string m_name;
    std::string m_type;
    std::string m_description;
    bool m_enabled {false};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    END_SERIAL_MEMEBER
};

struct Services {
    std::vector<ServiceInfo> m_services;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_services, services)
    END_SERIAL_MEMEBER
};

struct AppClusterInfo {
    AppClusterInfo(const ServiceInfo& service, std::string registerService, std::string cpsIp)
        : m_serviceId(service.m_id),
          m_serviceName(service.m_name),
          m_serviceType(service.m_type),
          m_registerService(registerService),
          m_cpsIp(cpsIp)
    {}
    std::string m_serviceId;
    std::string m_serviceName;
    std::string m_serviceType;
    std::string m_registerService;
    std::string m_cpsIp;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serviceId, service_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serviceName, service_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serviceType, service_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_registerService, register_service)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cpsIp, cps_ip)
    END_SERIAL_MEMEBER
};

struct ZoonState {
    bool available;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(available, available)
    END_SERIAL_MEMEBER
};

struct AvailabilityZone {
    std::string zoneName;
    ZoonState zoneState;
    std::string hosts;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(zoneName, zoneName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(zoneState, zoneState)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hosts, hosts)
    END_SERIAL_MEMEBER
};

struct AvailabilityZoneInfo {
    AvailabilityZone availabilityZone;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(availabilityZone, availabilityZone)
    END_SERIAL_MEMEBER
};

struct AvailabilityZoneInfoList {
    std::vector<AvailabilityZone> m_availabilityZoneInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_availabilityZoneInfo, availabilityZoneInfo)
    END_SERIAL_MEMEBER
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif
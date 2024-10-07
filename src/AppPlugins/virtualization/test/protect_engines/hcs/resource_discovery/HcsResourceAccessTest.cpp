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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <string.h>
#include <iostream>
#include <map>
#include <log/Log.h>
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "protect_engines/hcs/api/ecs/EcsClient.h"
#include "protect_engines/hcs/utils/StorageMgr.h"
#include "volume_handlers/common/ControlDevice.h"
#include "common/httpclient/HttpClient.h"
#include "common/CommonMock.h"
#include "config_reader/ConfigIniReader.h"
#include "protect_engines/hcs/resource_discovery/HcsResourceAccess.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;
using namespace HcsPlugin;

namespace HDT_TEST {

bool HCSStubGetToken(void *obj, ModelBase &model, std::string &tokenValue, std::string &endPoint) {
	tokenValue = "stubtokenstring";
	endPoint = "https://identity.az236.dc236.huawei.com/identity/v3";
	return true;
}

class HcsResourceAccessTest : public testing::Test {
protected:
    void InitLogger()
    {
        std::string logFileName = "virt_plugin_hcs_resource_access_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
                logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
    void SetUp()
    {
        /*app*/
        Authentication appAuth;
        appAuth.__set_authkey("admin");
        appAuth.__set_authPwd("passwd");
        appAuth.__set_extendInfo(
            "{\"vdcInfos\" : \"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei\\\"}]\"}");

        m_appInfo.__set_type("type");
        m_appInfo.__set_subType("subType");
        m_appInfo.__set_id("id");
        m_appInfo.__set_name("name");
        m_appInfo.__set_parentId("parentId");
        m_appInfo.__set_parentName("parentName");
        m_appInfo.__set_auth(appAuth);
        m_appInfo.__set_extendInfo("{\"domainId\":\"domain\"}");

        /*env*/
        Authentication envAuth;
        envAuth.__set_authkey("admin");
        envAuth.__set_authPwd("passwd");

        ApplicationEnvironment appEnv;
        m_appEnv.__set_id("envId");
        m_appEnv.__set_name("envName");
        m_appEnv.__set_type("envType");
        m_appEnv.__set_subType("envSubType");
        m_appEnv.__set_endpoint("demo.com");
        m_appEnv.__set_port(8088);
        m_appEnv.__set_auth(envAuth);
        std::vector<ApplicationEnvironment> nodes = {};
        m_appEnv.__set_nodes(nodes);
        m_appEnv.__set_extendInfo("");
        m_stub.set(sleep, Stub_Sleep);
        m_stub.set(ADDR(BaseTokenMgr, GetToken), HCSStubGetToken);
        InitLogger();
    }
    void TearDown()
    {
        m_stub.reset(sleep);
        m_stub.reset(ADDR(BaseTokenMgr, GetToken));
    }
    HcsResourceAccess GetTestInstanceWhenTaskParamOK()
    {
        Application appInfo = m_appInfo;
        std::string extendInfo =
            "{\"projectId\":\"\",\"regionId\":\"\",\"project\": "
            "\"{\\\"id\\\":\\\"\\\",\\\"name\\\":\\\"\\\",\\\"tenantId\\\":\\\"\\\",\\\"tenantName\\\":\\\"\\\","
            "\\\"regions\\\":[{\\\"regionId\\\":\\\"\\\",\\\"regionName\\\":\\\"\\\",\\\"regionStatus\\\":\\\"\\\"}],"
            "\\\"vdcInfo\\\":{\\\"name\\\":\\\"\\\",\\\"passwd\\\":\\\"\\\",\\\"domainName\\\":\\\"\\\"}}\"}";
        appInfo.__set_extendInfo(extendInfo);
        ApplicationEnvironment appEnv = m_appEnv;
        QueryByPage pageInfo;
        HcsResourceAccess hcsResourceAccess(appEnv, appInfo, pageInfo);
        return hcsResourceAccess;
    }

    HcsResourceAccess GetTestInstanceWhenTaskParamError()
    {
        Application appInfo = m_appInfo;
        std::string extendInfo = "{\"projectId\":\"\",\"regionId\":\"\"}";
        appInfo.__set_extendInfo(extendInfo);
        ApplicationEnvironment appEnv = m_appEnv;
        QueryByPage pageInfo;
        HcsResourceAccess hcsResourceAccess(appEnv, appInfo, pageInfo);
        return hcsResourceAccess;
    }

    Application m_appInfo;
    ApplicationEnvironment m_appEnv;
    Stub m_stub;
};

const std::string g_resVdcListBody = "{\"total\":3,\"vdcs\":[{\"id\":\"99076361b95f4226b18db0001555bd00\",\"name\":\"huangrong\",\"tag\":\"vdc\",\"description\":\"\",\"upper_vdc_id\":\"-1\",\"upper_vdc_name\":null,\"top_vdc_id\":\"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",\"extra\":\"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",\"ecs_used\":0.0,\"evs_used\":0.0,\"project_count\":0,\"enabled\":true,\"domain_id\":\"99076361b95f4226b18db0001555bd00\",\"level\":0,\"create_user_id\":\"da5b034745aa4db0a14fe57a86fb11f6\",\"create_user_name\":\"bss_admin\",\"create_at\":1652175583000,\"utc_create_at\":\"2022-05-10 09:39:43.0\",\"domain_name\":\"huangrong\",\"ldap_id\":null,\"third_id\":null,\"idp_name\":null,\"third_type\":\"0\",\"region_id\":null,\"enterprise_id\":null,\"az_id\":null,\"enterprise_project_id\":null},{\"id\":\"e283a3f9e0a64487934941674b4acd3a\",\"name\":\"oceanprotect\",\"tag\":\"vdc\",\"description\":\"\",\"upper_vdc_id\":\"-1\",\"upper_vdc_name\":null,\"top_vdc_id\":\"63a67934-909f-42b0-8363-82aba8c58ceb\",\"extra\":\"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",\"ecs_used\":0.0,\"evs_used\":0.0,\"project_count\":0,\"enabled\":true,\"domain_id\":\"e283a3f9e0a64487934941674b4acd3a\",\"level\":0,\"create_user_id\":\"da5b034745aa4db0a14fe57a86fb11f6\",\"create_user_name\":\"bss_admin\",\"create_at\":1654678668000,\"utc_create_at\":\"2022-06-08 08:57:48.0\",\"domain_name\":\"oceanprotect\",\"ldap_id\":null,\"third_id\":null,\"idp_name\":null,\"third_type\":\"0\",\"region_id\":null,\"enterprise_id\":null,\"az_id\":null,\"enterprise_project_id\":null},{\"id\":\"d28507a101364296b2e04ad44ff19777\",\"name\":\"test001\",\"tag\":\"vdc\",\"description\":\"\",\"upper_vdc_id\":\"-1\",\"upper_vdc_name\":null,\"top_vdc_id\":\"0ca22258-dbd9-4fdd-b482-37ca21330289\",\"extra\":\"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",\"ecs_used\":0.0,\"evs_used\":0.0,\"project_count\":0,\"enabled\":true,\"domain_id\":\"d28507a101364296b2e04ad44ff19777\",\"level\":0,\"create_user_id\":\"da5b034745aa4db0a14fe57a86fb11f6\",\"create_user_name\":\"bss_admin\",\"create_at\":1654761100000,\"utc_create_at\":\"2022-06-09 07:51:40.0\",\"domain_name\":\"test001\",\"ldap_id\":null,\"third_id\":null,\"idp_name\":null,\"third_type\":\"0\",\"region_id\":null,\"enterprise_id\":null,\"az_id\":null,\"enterprise_project_id\":null}]}";
const std::string g_resTokenBody = "{\"token\":{\"expires_at\":\"2022-07-22T07:59:59.869000Z\",\"methods\":[\"password\"],\"catalog\":[{\"endpoints\":[{\"region_id\":\"sc-cd-1\",\"id\":\"c8476fa3de214d42b99cdc8103b36a5e\",\"region\":\"sc-cd-1\",\"interface\":\"public\",\"url\":\"https://evs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"}],\"name\":\"evs\",\"id\":\"2cd3de7abbeb40fa859d73ab6a8282f5\",\"type\":\"volume\"},{\"endpoints\":[{\"region_id\":\"sc-cd-1\",\"id\":\"c1bf27b0c6644301971643034e063047\",\"region\":\"sc-cd-1\",\"interface\":\"internal\",\"url\":\"https://iam-cache-proxy.sc-cd-1.demo.com:26335\"},{\"region_id\":\"sc-cd-1\",\"id\":\"851c1913e6bc4c90ac3b948c7d20e665\",\"region\":\"sc-cd-1\",\"interface\":\"public\",\"url\":\"https://iam-apigateway-proxy.sc-cd-1.demo.com\"}],\"name\":\"iam\",\"id\":\"4b47e15c6c0c4d02b1b6ff75a003e6c6\",\"type\":\"iam\"},{\"endpoints\":[{\"region_id\":\"sc-cd-1\",\"id\":\"c4e4278a31014b6282d9a3403214b540\",\"region\":\"sc-cd-1\",\"interface\":\"public\",\"url\":\"https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"}],\"name\":\"ecs\",\"id\":\"b1efa676dfad47609c7b9a6a6fe2c861\",\"type\":\"compute\"},{\"endpoints\":[{\"region_id\":\"sc-cd-1\",\"id\":\"cd044271e54348c0b4f1cb14211b317b\",\"region\":\"sc-cd-1\",\"interface\":\"internal\",\"url\":\"https://sc.demo.com:26335\"},{\"region_id\":\"sc-cd-1\",\\\"id\":\"75b86267698a4cb3a34e2a1c555ee77f\",\"region\":\"sc-cd-1\",\"interface\":\"public\",\"url\":\"https://sc.demo.com\"}],\"name\":\"sc\",\"id\":\"b921c288b2314f5fa13bd9342823e6d9\",\"type\":\"sc\"},],\"roles\":[{\"name\":\"vdc_adm\",\"id\":\"ca71e771bafc42999098f088a784c751\"},{\"name\":\"tag_adm\",\"id\":\"d6bb5dbf74b44b95aaf2e70bfc72514f\"},],\"project\":{\"domain\":{\"name\":\"huangrong\",\"id\":\"99076361b95f4226b18db0001555bd00\"},\"name\":\"sc-cd-1_test\",\"id\":\"e38d227edcce4631be20bfa5aad7130b\"},\"issued_at\":\"2022-07-21T07:59:59.869000Z\",\"user\":{\"domain\":{\"name\":\"huangrong\",\"id\":\"99076361b95f4226b18db0001555bd00\"},\"name\":\"huangrong\",\"id\":\"d4216b7d3ba64a4eb63db37c2b91222c\"}}}";
const std::string g_resProjectListBody = "{\"total\":1,\"projects\":[{\"id\":\"e38d227edcce4631be20bfa5aad7130b\",\"name\":\"sc-cd-1_test\",\"description\":\"\",\"domain_id\":\"99076361b95f4226b18db0001555bd00\",\"enabled\":true,\"tenant_id\":\"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",\"is_shared\":false,\"tenant_name\":\"huangrong\",\"create_user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"create_user_name\":\"huangrong\",\"regions\":[{\"region_id\":\"sc-cd-1\",\"region_name\":{\"zh_cn\":\"西南\",\"en_us\":\"西南\"},\"region_type\":null,\"region_status\":\"normal\"}]}]}";
const std::string g_resProjectDetailBody = "{\"project\":{\"tenant_id\":\"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",\"create_user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"tenant_name\":\"huangrong\",\"create_user_name\":\"huangrong\",\"description\":\"\",\"tenant_type\":\"vdc\",\"enabled\":true,\"domain_id\":\"99076361b95f4226b18db0001555bd00\",\"contract_number\":null,\"is_shared\":false,\"name\":\"sc-cd-1_test\",\"iam_project_name\":\"sc-cd-1_test\",\"display_name\":\"sc-cd-1_test\",\"id\":\"e38d227edcce4631be20bfa5aad7130b\",\"owner_id\":null,\"owner_name\":null,\"region_name\":null,\"regions\":[{\"region_id\":\"sc-cd-1\",\"region_name\":{\"zh_cn\":\"西南\",\"en_us\":\"西南\"},\"region_type\":null,\"region_status\":\"normal\",\"cloud_infras\":[{\"cloud_infra_id\":\"FUSION_CLOUD_sc-cd-1\",\"cloud_infra_name\":\"OpenStack_sc-cd-1\",\"cloud_infra_status\":\"normal\",\"cloud_infra_type\":\"FUSION_CLOUD\",\"azs\":[{\"az_id\":\"az0.dc0\",\"az_name\":\"某某公司\",\"az_status\":\"normal\"}],\"quotas\":[]}]}],\"attachment_id\":null,\"attachment_name\":null,\"attachment_size\":0,\"is_support_hws_service\":true}}";
const std::string g_ecs_serverList = "{\"count\":3,\"servers\":[{\"id\":\"c982522d-c5ec-44f7-9919-25bb1587a48f\",\"name\":\"ecs-4ac8-0005\"},{\"id\":\"f0e97318-3ff6-49b8-90c3-6d5d1367af8b\",\"name\":\"ecs-4ac8-0002\"},{\"id\":\"660e57b4-e59d-4aa1-9be4-04ca179d67c5\",\"name\":\"bb-001\"}]}";
const std::string g_ecs_server_detail =
    "{\"server\":{\"tenant_id\":\"e38d227edcce4631be20bfa5aad7130b\",\"addresses\":{\"subnet-8f61\":[{\"OS-EXT-IPS-MAC:"
    "mac_addr\":\"fa:16:3e:e0:82:94\",\"OS-EXT-IPS:type\":\"fixed\",\"addr\":\"192.168.0.179\",\"version\":4}]},"
    "\"metadata\":{\"productId\":\"5b4ecaa32947446b824df4a6c60c8a04\",\"__instance_vwatchdog\":\"false\",\"_ha_policy_"
    "type\":\"remote_rebuild\",\"server_expiry\":\"0\",\"cascaded.instance_extrainfo\":\"system_serial_number:17af2355-"
    "5865-43a8-af30-59c82c9d49e5,max_mem:4194304,max_cpu:254,current_mem:8192,cpu_num_for_one_plug:1,org_cpu:4,xml_"
    "support_live_resize:True,num_of_mem_plug:57,org_mem:8192,iohang_timeout:720,current_cpu:4,uefi_mode_sysinfo_"
    "fields:version_serial_uuid_family_asset,pcibridge:2\"},\"OS-EXT-STS:task_state\":null,\"OS-DCF:diskConfig\":"
    "\"MANUAL\",\"OS-EXT-AZ:availability_zone\":\"az0.dc0\",\"links\":[{\"rel\":\"self\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b/servers/"
    "17af2355-5865-43a8-af30-59c82c9d49e5\"},{\"rel\":\"bookmark\",\"href\":\"https://ecs.sc-cd-1.demo.com/"
    "e38d227edcce4631be20bfa5aad7130b/servers/"
    "17af2355-5865-43a8-af30-59c82c9d49e5\"}],\"OS-EXT-STS:power_state\":1,\"id\":\"17af2355-5865-43a8-af30-"
    "59c82c9d49e5\",\"os-extended-volumes:volumes_attached\":[{\"id\":\"dd28a56b-014c-4c05-bc53-9fbf0db78788\"}],\"OS-"
    "EXT-SRV-ATTR:host\":\"EA918B93-2561-E611-9B2A-049FCAD22DFC\",\"image\":{\"links\":[{\"rel\":\"bookmark\",\"href\":"
    "\"https://ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/images/"
    "8ea95ea3-72ae-4e01-8552-2df5a5140a52\"}],\"id\":\"8ea95ea3-72ae-4e01-8552-2df5a5140a52\"},\"OS-SRV-USG:terminated_"
    "at\":null,\"accessIPv4\":\"\",\"accessIPv6\":\"\",\"created\":\"2022-07-20T09:55:05Z\",\"hostId\":"
    "\"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\"OS-EXT-SRV-ATTR:hypervisor_hostname\":\"EA918B93-"
    "2561-E611-9B2A-049FCAD22DFC\",\"key_name\":null,\"flavor\":{\"links\":[{\"rel\":\"bookmark\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/flavors/"
    "ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"}],\"id\":\"ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"},\"security_groups\":[{"
    "\"name\":\"default\"}],\"config_drive\":\"\",\"OS-EXT-STS:vm_state\":\"active\",\"OS-EXT-SRV-ATTR:instance_name\":"
    "\"instance-00000039\",\"user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"name\":\"HCS-1\",\"progress\":0,\"OS-SRV-"
    "USG:launched_at\":\"2022-07-20T09:55:17.000000\",\"updated\":\"2022-07-20T10:06:14Z\",\"status\":\"ACTIVE\"}}";

const std::string g_evs_disk_detail =
    "{\"volume\":{\"id\":\"dd28a56b-014c-4c05-bc53-9fbf0db78788\",\"links\":[{\"href\":\"https://evs.sc-cd-1.demo.com/"
    "v2/e38d227edcce4631be20bfa5aad7130b/volumes/"
    "dd28a56b-014c-4c05-bc53-9fbf0db78788\",\"rel\":\"self\"},{\"href\":\"https://evs.sc-cd-1.demo.com/"
    "e38d227edcce4631be20bfa5aad7130b/volumes/"
    "dd28a56b-014c-4c05-bc53-9fbf0db78788\",\"rel\":\"bookmark\"}],\"name\":\"HCS-1-volume-0000\",\"status\":\"in-"
    "use\",\"attachments\":[{\"server_id\":\"17af2355-5865-43a8-af30-59c82c9d49e5\",\"attachment_id\":\"807d59dc-985f-"
    "48b5-b549-4624e6b12125\",\"attached_at\":\"2022-07-20T09:55:13.187039\",\"host_name\":null,\"volume_id\":"
    "\"dd28a56b-014c-4c05-bc53-9fbf0db78788\",\"device\":\"/dev/"
    "vda\",\"id\":\"dd28a56b-014c-4c05-bc53-9fbf0db78788\"}],\"availability_zone\":\"az0.dc0\",\"os-vol-host-attr:"
    "host\":\"cinder-kvm002@typei_global_bussiness_01#StoragePool003-bu\",\"source_volid\":null,\"snapshot_id\":null,"
    "\"description\":null,\"created_at\":\"2022-07-20T09:53:28.915805\",\"volume_type\":\"business_type_01\",\"os-vol-"
    "tenant-attr:tenant_id\":\"e38d227edcce4631be20bfa5aad7130b\",\"size\":10,\"metadata\":{\"take_over_lun_wwn\":\"--"
    "\",\"tenancy\":\"0\",\"lun_wwn\":\"658f987100b749bc2052749a000000bd\",\"StorageType\":\"OceanStorV5\",\"__sys_is_"
    "server_vol__\":\"true\",\"readonly\":\"False\",\"attached_mode\":\"rw\"},\"volume_image_metadata\":{\"size\":"
    "\"1492910080\",\"ori_disk_format\":\"qcow2\",\"file_name\":\"**.qcow2\",\"min_ram\":\"0\",\"cloudinit\":\"False\","
    "\"image_name\":\"HCS-1\",\"hw_firmware_type\":\"bios\",\"image_id\":\"8ea95ea3-72ae-4e01-8552-2df5a5140a52\",\"__"
    "admin_encrypted\":\"false\",\"__os_type\":\"Linux\",\"__os_bit\":\"64\",\"__data_origin\":\"instance,b85880d7-"
    "65c2-4c0d-bb27-c3d8e7e1b5b8\",\"__support_kvm\":\"true\",\"virtual_env_type\":\"FusionCompute\",\"__system_"
    "encrypted\":\"false\",\"expired_at\":\"0\",\"min_disk\":\"10\",\"__os_version\":\"Astra Linux 1.4 "
    "64bit\",\"__support_live_resize\":\"False\",\"__support_static_ip\":\"False\",\"file_format\":\"qcow2\","
    "\"checksum\":\"c04dd2d7951dde54ec3546610cccbc58\",\"__imagetype\":\"private\",\"disk_format\":\"qcow2\",\"is_auto_"
    "config\":\"false\",\"__platform\":\"Other\",\"architecture\":\"x86_64\",\"container_format\":\"bare\",\"__"
    "isregistered\":\"true\",\"__virtual_size\":\"10\",\"hw_disk_bus\":\"virtio\"},\"os-vol-mig-status-attr:migstat\":"
    "null,\"os-vol-mig-status-attr:name_id\":null,\"encrypted\":false,\"replication_status\":\"disabled\",\"user_id\":"
    "\"d4216b7d3ba64a4eb63db37c2b91222c\",\"consistencygroup_id\":null,\"bootable\":\"true\",\"updated_at\":\"2022-07-"
    "20T09:55:13.230110\",\"shareable\":false,\"multiattach\":false,\"os-volume-replication:extended_status\":null,"
    "\"os-volume-replication:driver_data\":\"{\\\"lun_id\\\": \\\"189\\\", \\\"sn\\\": "
    "\\\"2102351NPT10J3000001\\\"}\"}}";

const std::string g_evsDiskListDetail =
    "{\"count\": 2,\
    \"volumes\": [{\"attachments\": [{\
                    \"server_id\": \"78f14666-1309-421c-ae3b-60649c616b92\",\
                    \"attachment_id\": \"08ba7927-b5f9-4e95-982e-bdb0d77751d6\",\
                    \"attached_at\": \"2022-10-19T11:50:26.035705\",\
                    \"volume_id\": \"6682771a-0214-4bab-8699-b54e4815d439\",\
                    \"device\": \"/dev/sdc\",\
                    \"id\": \"6682771a-0214-4bab-8699-b54e4815d439\"}],\
            \"wwn\": \"658f987100b749bc10fb2c7800000630\",\
            \"id\": \"6682771a-0214-4bab-8699-b54e4815d439\",\
            \"size\": 30,\
            \"service_type\": \"EVS\",\
            \"os-vol-mig-status-attr:migstat\": null,\
            \"metadata\": {\
                \"take_over_lun_wwn\": \"--\",\
                \"lun_wwn\": \"658f987100b749bc10fb2c7800000630\",\
                \"StorageType\": \"OceanStorV5\",\
                \"readonly\": \"False\",\
                \"hw:passthrough\": \"true\"},\
            \"status\": \"in-use\",\
            \"name\": \"rigoujian-volume-0001\",\
            \"bootable\": \"false\",\
            \"shareable\": false\
        },\
        {\"attachmentss\": [{\
                    \"server_id\": \"78f14666-1309-421c-ae3b-60649c616b92\",\
                    \"attachment_id\": \"08ba7927-b5f9-4e95-982e-bdb0d77751d6\",\
                    \"attached_at\": \"2022-10-19T11:50:26.035705\",\
                    \"volume_id\": \"6682771a-0214-4bab-8699-b54e4815d439\",\
                    \"device\": \"/dev/sdc\",\
                    \"id\": \"6682771a-0214-4bab-8699-b54e4815d439\"}],\
            \"wwn\": \"658f987100b749bc10fb2c7800000630\",\
            \"id\": \"6682771a-0214-4bab-8699-b54e4815d439\",\
            \"size\": 20,\
            \"service_type\": \"EVS\",\
            \"os-vol-mig-status-attr:migstat\": null,\
            \"metadata\": {\
                \"take_over_lun_wwn\": \"--\",\
                \"lun_wwn\": \"658f987100b749bc10fb2c7800000630\",\
                \"StorageType\": \"OceanStorV5\",\
                \"readonly\": \"False\",\
                \"hw:passthrough\": \"true\"},\
            \"status\": \"in-use\",\
            \"name\": \"rigoujian-volume-0002\",\
            \"bootable\": \"false\",\
            \"shareable\": false\
        }]}";

const std::string g_mutiHcsStorageAuthExtendInfo =
    "{ 	\"enableCert\" : \"0\", 	\"storages\" : "
    "\"[{\\\"username\\\":\\\"zl_admin\\\",\\\"password\\\":\\\"admin@storage4\\\",\\\"ip\\\":\\\"88.1.1.21\\\","
    "\\\"port\\\":8088,\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":\\\"\\\",\\\"revocationList\\\":\\\"\\\","
    "\\\"certName\\\":\\\"\\\",\\\"certSize\\\":\\\"\\\",\\\"crlName\\\":\\\"\\\",\\\"crlSize\\\":\\\"\\\",\\\"sn\\\":"
    "\\\"88.1.1.21\\\",\\\"storageType\\\":\\\"1\\\"},{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"Admin@"
    "1234\\\",\\\"ip\\\":\\\"8.40.99.81\\\",\\\"port\\\":8088,\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":"
    "\\\"\\\",\\\"revocationList\\\":\\\"\\\",\\\"certName\\\":\\\"\\\",\\\"certSize\\\":\\\"\\\",\\\"crlName\\\":"
    "\\\"\\\",\\\"crlSize\\\":\\\"\\\",\\\"sn\\\":\\\"2102351NPT10J3000001\\\",\\\"storageType\\\":\\\"0\\\"}]\", 	"
    "\"vdcInfo\" : "
    "\"{\\\"name\\\":\\\"admin_zl\\\",\\\"passwd\\\":\\\"admin@storage1\\\",\\\"domainName\\\":\\\"zl_hcs_test\\\"}\" "
    "}";

std::shared_ptr<QueryVdcListResponse> Stub_ScClient_QueryVdcList_Success(QueryVdcListRequest &request)
{
    std::shared_ptr<QueryVdcListResponse> response = std::make_shared<QueryVdcListResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    response->SetGetBody(g_resVdcListBody);
    return response;
}

std::shared_ptr<QueryVdcListResponse> Stub_ScClient_QueryVdcList_Failed(QueryVdcListRequest &request)
{
    std::shared_ptr<QueryVdcListResponse> response = std::make_shared<QueryVdcListResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::NOT_FOUND));
    return response;
}

std::shared_ptr<QueryProjectDetailResponse> Stub_ScClient_QueryProjectDetail_Success(QueryProjectDetailRequest &request)
{
    std::shared_ptr<QueryProjectDetailResponse> response = std::make_shared<QueryProjectDetailResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    response->SetGetBody(g_resProjectDetailBody);
    return response;
}

std::shared_ptr<QueryProjectDetailResponse> Stub_ScClient_QueryProjectDetail_Failed(QueryProjectDetailRequest &request)
{
    std::shared_ptr<QueryProjectDetailResponse> response = std::make_shared<QueryProjectDetailResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::NOT_FOUND));
    return response;
}

std::shared_ptr<GetTokenResponse> Stub_IamClient_GetToken_Success(GetTokenRequest &request)
{
    std::shared_ptr<GetTokenResponse> response = std::make_shared<GetTokenResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
    response->SetGetBody(g_resTokenBody);
    return response;
}

std::shared_ptr<QueryResourceListResponse> Stub_ScClient_QueryResourceList_Success(QueryResourceListRequest &request)
{
    std::shared_ptr<QueryResourceListResponse> response = std::make_shared<QueryResourceListResponse>();
    response->SetHttpStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    response->SetGetBody(g_resProjectListBody);
    return response;
}

std::shared_ptr<QueryResourceListResponse> Stub_ScClient_QueryResourceList_Failed(QueryResourceListRequest &request)
{
    std::shared_ptr<QueryResourceListResponse> response = std::make_shared<QueryResourceListResponse>();
    response->SetHttpStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::NOT_FOUND));
    return response;
}

/*
 * 测试用例： 查询租户成功
 * 前置条件： 不对request设置参数
 * CHECK点： reponse返回值为nullptr
 */
TEST_F(HcsResourceAccessTest, GetTenantInfoSuccess)
{
    QueryByPage pageInfo;
    pageInfo.__set_pageNo(1);
    pageInfo.__set_pageSize(1);
    ApplicationEnvironment appEnv = m_appEnv;
    HcsResourceAccess hcsResourceAccess(appEnv, pageInfo);

    Stub stub;
    stub.set(ADDR(ScClient, QueryVdcList), Stub_ScClient_QueryVdcList_Success);
    ResourceResultByPage page;
    int ret = hcsResourceAccess.GetTenantInfo(page);
    stub.reset(ADDR(ScClient, QueryVdcList));
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 测试用例： 查询租户，pm下发参数错误，缺少projectid
 * 前置条件： 不对request设置参数
 * CHECK点： reponse返回值为nullptr
 */

TEST_F(HcsResourceAccessTest, GetTenantInfoFailed)
{
    QueryByPage pageInfo;
    pageInfo.__set_pageNo(1);
    pageInfo.__set_pageSize(1);
    ApplicationEnvironment appEnv = m_appEnv;
    HcsResourceAccess hcsResourceAccess(appEnv, pageInfo);

    Stub stub;
    stub.set(ADDR(ScClient, QueryVdcList), Stub_ScClient_QueryVdcList_Failed);
    ResourceResultByPage page;
    int ret = hcsResourceAccess.GetTenantInfo(page);
    stub.reset(ADDR(ScClient, QueryVdcList));
    EXPECT_EQ(ret, FAILED);
}

std::string g_vdcList = "{\
	\"total\": 1,\
	\"vdcs\": [{\
		\"id\": \"bc8d0ba9-c4e0-424a-b6c5-48432bd724c2\",\
		\"name\": \"ceshi-01\",\
		\"tag\": \"vdc\",\
		\"description\": null,\
		\"upper_vdc_id\": \"0\",\
		\"upper_vdc_name\": null,\
		\"top_vdc_id\": \"54da6195-789c-425e-9fe8-f692154bf271\",\
		\"extra\": \"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",\
		\"ecs_used\": 0.0,\
		\"evs_used\": 0.0,\
		\"project_count\": 0,\
		\"enabled\": true,\
		\"domain_id\": \"f037e0dd25134b8bab74e668cfa466ad\",\
		\"level\": 1,\
		\"create_user_id\": \"da5b034745aa4db0a14fe57a86fb11f6\",\
		\"create_user_name\": \"bss_admin\",\
		\"create_at\": 1661773634000,\
		\"utc_create_at\": \"2022-08-29 11:47:14.0\",\
		\"domain_name\": \"ceshi-01\",\
		\"ldap_id\": null,\
		\"third_id\": null,\
		\"idp_name\": null,\
		\"third_type\": \"0\",\
		\"region_id\": null,\
		\"enterprise_id\": null,\
		\"az_id\": null,\
		\"enterprise_project_id\": null\
	}]\
}";

std::string g_userDetail = "{\
    \"user\": {\
        \"enabled\": \"true\",\
        \"description\": \"\",\
        \"email\": \"\",\
        \"areacode\": null,\
        \"phone\": \"\",\
        \"projects\": [\
            {\
                \"id\": \"7f31f1c417734c96991366651c18d6a4\",\
                \"region_id\": \"sc-cd-1\",\
                \"name\": \"sc-cd-1_xjptest\",\
                \"iam_project_name\": \"sc-cd-1_xjptest\",\
                \"description\": \"\"\
            }\
        ],\
        \"ldap_id\": null,\
        \"level\": \"3\",\
        \"resource_tenant_id\": null,\
        \"roles\": [\
            {\
                \"id\": \"00000000-0000-0000-0000-000000000001\",\
                \"name\": \"vdcServiceManager\",\
                \"display_name\": \"VDC Admin\",\
                \"user_role_type\": null,\
                \"description\": \"role_role_view_para_desc_content_vdcServiceManager_value\"\
            }\
        ],\
        \"display_name\": \"\",\
        \"second_verify\": \"false\",\
        \"vdc_id\": \"bc8d0ba9-c4e0-424a-b6c5-48432bd724c2\",\
        \"domain_id\": \"f653b2d8a0f944949bd37eaa896d3564\",\
        \"domain_name\": \"youlei001\",\
        \"user_type\": \"0\",\
        \"name\": \"xujianping\",\
        \"login_at\": null,\
        \"id\": \"f0ae7f3e51c94b1497a9cbfc526d7af8\",\
        \"tag\": \"vdc\",\
        \"top_vdc_id\": \"fc1c573a-68af-4e08-96a7-39c7a219f97e\",\
    },\
    \"federation_regions\": []\
}";

int32_t Stub_SendRequestGetUserDetailSuccess(
    void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) {
        count += 1;
        response->SetSuccess(true);
        response->SetGetBody(g_vdcList);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    } else {
        response->SetSuccess(true);
        response->SetGetBody(g_userDetail);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    }
    return SUCCESS;
}

int32_t Stub_CheckAppConnect(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // get user detail
        count += 1;
        response->SetSuccess(true);
        response->SetGetBody(g_userDetail);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    } else if (count == 1) { // Get IAM token
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
    }
    return SUCCESS;
}

bool Stub_ResAccessGetTokenSuccess(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = "1234242";
    endPoint = "http://sc.demo.com/";
    return true;
}

/**
 * 测试用例：获取工程列表
 * 前置条件：token存在，工程存在
 * Check点：获取成功
 */
TEST_F(HcsResourceAccessTest, GetProjectLists_SUCC)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestGetUserDetailSuccess);
    stub.set(ADDR(BaseTokenMgr, GetToken), Stub_ResAccessGetTokenSuccess);
    Application appInfo;
    // appInfo.__set_extendInfo("{\"domainId\":\"youlei001\"}");
    // appInfo.auth.__set_extendInfo("{\"vdcInfos\":\"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei!@#$1234\\\"}]\"}");
    ApplicationEnvironment appEnv = m_appEnv;
    ResourceResultByPage page;
    HcsResourceAccess hcsResourceAccess(appEnv, appInfo);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domainId\":\"youlei001\"}");
    hcsResourceAccess.m_application.auth.__set_extendInfo("{\"vdcInfos\":\"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei!@#$1234\\\"}]\"}");
    std::string parentId;
    // stub.set(ADDR(ScClient, QueryVdcList), Stub_ScClient_QueryVdcList_Success);
    stub.set(ADDR(ScClient, QueryProjectDetail), Stub_ScClient_QueryProjectDetail_Success);
    stub.set(ADDR(IamClient, GetToken), Stub_IamClient_GetToken_Success);
    stub.set(ADDR(ScClient, QueryResourceList), Stub_ScClient_QueryResourceList_Success);
    int ret = hcsResourceAccess.GetProjectLists(page, parentId);
    stub.reset(ADDR(ScClient, QueryVdcList));
    stub.reset(ADDR(ScClient, QueryProjectDetail));
    stub.reset(ADDR(IamClient, GetToken));
    stub.reset(ADDR(ScClient, QueryResourceList));
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：获取工程列表
 * 前置条件：Vdc不存在
 * Check点：获取失败
 */
TEST_F(HcsResourceAccessTest, GetProjectLists_VdcNotExist_FAILED)
{
    Application appInfo = m_appInfo;
    ApplicationEnvironment appEnv = m_appEnv;
    ResourceResultByPage page;
    HcsResourceAccess hcsResourceAccess(appEnv, appInfo);
    std::vector<std::string> errorUserName;
    std::string parentId;

    Stub stub;
    stub.set(ADDR(ScClient, QueryVdcList), Stub_ScClient_QueryVdcList_Failed);
    int ret = hcsResourceAccess.GetVDCResourceList(page, errorUserName, parentId);
    stub.reset(ADDR(ScClient, QueryVdcList));
    EXPECT_EQ(ret, FAILED);
}

std::shared_ptr<OpenStackPlugin::GetProjectServersResponse> Stub_Get_ServerListSuccess(void *obj,
    GetProjectServersRequest &request)
{
    std::shared_ptr<OpenStackPlugin::GetProjectServersResponse> response = std::make_shared<OpenStackPlugin::GetProjectServersResponse>();
    response->SetStatusCode(200);
    response->SetGetBody(g_ecs_serverList);
    response->Serial();
    return response;
}

static std::shared_ptr<ShowVolumeDetailResponse> StubGetVolumeListDetialSuccess(
    void *obj, ShowVolumeListRequest &request)
{
    std::shared_ptr<ShowVolumeDetailResponse> response = std::make_shared<ShowVolumeDetailResponse>();
    response->SetStatusCode(200);
    response->SetGetBody(g_evsDiskListDetail);
    bool bisr = response->Serial();
    return response;
}

static std::shared_ptr<ShowVolumeDetailResponse> StubGetVolumeListDetialFailed(
    void *obj, ShowVolumeDetailResponse &request)
{
    std::shared_ptr<ShowVolumeDetailResponse> response = nullptr;
    return response;
}

static std::shared_ptr<ShowVolumeDetailResponse> StubGetVolumeListDetialFailedOtherStatus(
    void *obj, ShowVolumeListRequest &request)
{
    std::shared_ptr<ShowVolumeDetailResponse> response = std::make_shared<ShowVolumeDetailResponse>();
    response->SetStatusCode(401);
    response->SetGetBody(g_evsDiskListDetail);
    response->Serial();
    return response;
}

std::shared_ptr<OpenStackPlugin::GetProjectServersResponse> Stub_Get_ServerListFailed(void *obj, GetProjectServersRequest &request)
{
    std::shared_ptr<OpenStackPlugin::GetProjectServersResponse> response = std::make_shared<OpenStackPlugin::GetProjectServersResponse>();
    response->SetStatusCode(404);
    return response;
}

std::shared_ptr<GetServerDetailsResponse> Stub_Get_ServerDetailsSuccess(void *obj, GetServerDetailsRequest &request)
{
    std::shared_ptr<GetServerDetailsResponse> response = std::make_shared<GetServerDetailsResponse>();
    response->SetStatusCode(200);
    response->SetGetBody(g_ecs_server_detail);
    return response;
}

std::shared_ptr<GetServerDetailsResponse> Stub_Get_ServerDetailsFailed(void *obj, GetServerDetailsRequest &request)
{
    std::shared_ptr<GetServerDetailsResponse> response = std::make_shared<GetServerDetailsResponse>();
    response->SetStatusCode(404);
    return response;
}

std::shared_ptr<ShowVolumeResponse> Stub_Get_VolumeDetailsSuccess(void *obj, ShowVolumeRequest &request)
{
    std::shared_ptr<ShowVolumeResponse> response = std::make_shared<ShowVolumeResponse>();
    response->SetStatusCode(200);
    response->SetGetBody(g_evs_disk_detail);
    return response;
}

std::shared_ptr<ShowVolumeResponse> Stub_Get_VolumeDetailsFailed(void *obj, ShowVolumeRequest &request)
{
    std::shared_ptr<ShowVolumeResponse> response = std::make_shared<ShowVolumeResponse>();
    response->SetStatusCode(404);
    return response;
}

static int32_t StubGetSpcifiedEVSDiskOK(DiskInfo &diskInfo, const std::string &diskId)
{
    diskInfo.m_shareable = true;
    return SUCCESS;
}

static bool StubGetVdcsInfoUnderTenantByScApiSuccess(HcsResourceAccess* obj, VdcListDetail& vdcList,
    bool needTry = true)
{
    VdcList vdcitem;
    vdcitem.m_id = "570882f0-87cf-4135-956c-f002e089bb77";
    vdcitem.m_name = "admin";
    vdcitem.m_tag = "";
    vdcitem.m_description = "";
    vdcitem.m_upperVdcId = "";
    vdcitem.m_upperVdcName = "";
    vdcitem.m_topVdcId = "";
    vdcitem.m_extra = "";
    vdcitem.m_projectCount = 1;
    vdcitem.m_domainId = "";
    vdcitem.m_createUserId = "";
    vdcitem.m_createUserName = "";
    vdcitem.m_utcCreateAt = "";
    vdcitem.m_domainName = "";
    vdcitem.m_ldapId = "";
    vdcitem.m_thirdId = "";
    vdcitem.m_idpName = "";
    vdcitem.m_thirdType = "";
    vdcitem.m_regionId = "";
    vdcitem.m_enterpriseId = "";
    vdcitem.m_azId = "";
    vdcitem.m_enterpriseProjectId = "";
    vdcitem.m_ecsUsed = 0.0;
    vdcitem.m_evsUsed = 0.0;
    vdcitem.m_enabled = true;
    vdcitem.m_level = 1;
    vdcitem.m_createAt = 0;
    vdcList.m_total = 1;
    vdcList.m_vdcs.clear();
    vdcList.m_vdcs.push_back(vdcitem);
    return true;
}

static bool StubSerialFailed()
{
    return false;
}

static bool StubSerialSuccess()
{
    return true;
}

static std::shared_ptr<GetTokenResponse> IamStubGetTokenSuccess(IamClient* obj, GetTokenRequest &request)
{
    std::shared_ptr<GetTokenResponse> response = std::make_shared<GetTokenResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
    return response;
}

static std::shared_ptr<GetTokenResponse> IamStubGetTokenFailed(IamClient* obj, GetTokenRequest &request)
{
    std::shared_ptr<GetTokenResponse> response = nullptr;
    return response;
}

static std::shared_ptr<QueryVdcListResponse> StubQueryVdcListSuccess(ScClient* obj, QueryVdcListRequest &request)
{
    std::shared_ptr<QueryVdcListResponse> response = std::make_shared<QueryVdcListResponse>();
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    StubGetVdcsInfoUnderTenantByScApiSuccess(nullptr, response->m_vdcListDetail, true);
    return response;
}

static bool StubGetStorageSystemInfoTrue(const ControlDeviceInfo &info, StorageSysInfo &storageSysInfo,
    std::string &errorStr)
{
    return true;
}

static bool StubGetStorageSystemInfoFalse(const ControlDeviceInfo &info, StorageSysInfo &storageSysInfo,
    std::string &errorStr)
{
    return false;
}

const std::string g_projectId = "e38d227edcce4631be20bfa5aad7130b";

const std::string g_tokenStr = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

const std::string g_endpoint = "https://compute.az0.dc0.demo.com:443/v2/e38d227edcce4631be20bfa5aad7130b";

const std::string g_serverListDetail =
    "{\"count\": 1,\
    \"servers\": [{\
    \"id\": \"78f14666-1309-421c-ae3b-60649c616b92\",\
    \"name\": \"rigoujian-勿动\",\
    \"status\": \"ACTIVE\",\
    \"progress\": 0,\
    \"updated\": \"2022-10-19T11:50:54Z\",\
    \"created\": \"2022-10-17T07:28:08Z\",\
    \"metadata\": {\
    \"_ha_policy_type\": \"remote_rebuild\",\
    \"metering.imagetype\": \"gold\",\
    \"vpc_id\": \"4b92c9ca-5025-44ca-9779-b5e17e029ff5\"},\
    \"key_name\": null,\
    \"OS-EXT-STS:task_state\": null,\
    \"OS-EXT-STS:power_state\": 1}]}";

static bool StubGetTokenSuccess(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = g_tokenStr;
    endPoint = g_endpoint;
    return true;
}

static bool StubGetTokenFailed(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    return false;
}

static Module::IHttpClient *StubGetServersDetailListSuccess()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_serverListDetail));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *StubSendRequestFailed()
{   
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(404));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(404));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}


/*
 * 测试用例： 查询主机列表成功
 * 前置条件： 调用hcs接口成功
 * CHECK点： 1.GetVirtualMachineList返回值为SUCCESS;2.主机列表和打桩一执
 */
TEST_F(HcsResourceAccessTest, GetHostListSuccess)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    hcsResourceAccess.m_condition.conditions = "{\"sourceType\":\"vm\"}";
    Stub stub;
    stub.set(ADDR(HcsNovaClient, GetProjectServers), Stub_Get_ServerListSuccess);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVirtualMachineList(page, parentId);
    stub.reset(ADDR(HcsNovaClient, GetProjectServers));
    std::vector<std::string> vmLists = {"c982522d-c5ec-44f7-9919-25bb1587a48f",
        "f0e97318-3ff6-49b8-90c3-6d5d1367af8b",
        "660e57b4-e59d-4aa1-9be4-04ca179d67c5"};
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(page.total, 3);
}

/*
 * 测试用例： 查询主机列表失败
 * 前置条件： 调用hcs接口失败
 * CHECK点： GetVirtualMachineList返回值为FAILED
 */
TEST_F(HcsResourceAccessTest, GetHostListFailed)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    hcsResourceAccess.m_condition.conditions = "{\"sourceType\":\"vm\"}";
    Stub stub;
    stub.set(ADDR(HcsNovaClient, GetProjectServers), Stub_Get_ServerListFailed);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVirtualMachineList(page, parentId);
    stub.reset(ADDR(HcsNovaClient, GetProjectServers));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询主机列表失败
 * 前置条件： 打桩pm下发参数错误，缺少project
 * CHECK点： GetVirtualMachineList返回值为FAILED
 */
TEST_F(HcsResourceAccessTest, GetHostListFailed_taskParamError)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamError();
    hcsResourceAccess.m_condition.conditions = "{\"sourceType\":\"vm\"}";
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVirtualMachineList(page, parentId);
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询磁盘列表信息成功
 * 前置条件： 调用hcs接口成功
 * CHECK点：  调用接口返回SUCCESS
 */
TEST_F(HcsResourceAccessTest, GetVolumeResourceListSuccess)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    Stub stub;
    stub.set(ADDR(EvsClient, ShowVolumeDetailList), StubGetVolumeListDetialSuccess);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVolumeResourceList(page, parentId);
    stub.reset(ADDR(EvsClient, ShowVolumeDetailList));
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(page.total, 2);
}

/*
 * 测试用例： 查询磁盘列表信息
 * 前置条件： 扩展信息校验失败
 * CHECK点：  调用查询磁盘列表信息失败
 */
TEST_F(HcsResourceAccessTest, GetVolumeResourceListFailedParamsFailed)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamError();
    Stub stub;
    stub.set(ADDR(EvsClient, ShowVolumeDetailList), StubGetVolumeListDetialSuccess);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVolumeResourceList(page, parentId);
    stub.reset(ADDR(EvsClient, ShowVolumeDetailList));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询磁盘列表信息
 * 前置条件： 调用EVS获取失败，response为空
 * CHECK点：  调用查询磁盘列表信息失败
 */
TEST_F(HcsResourceAccessTest, GetVolumeResourceListFailedResponseNull)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    Stub stub;
    stub.set(ADDR(EvsClient, ShowVolumeDetailList), StubGetVolumeListDetialFailed);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVolumeResourceList(page, parentId);
    stub.reset(ADDR(EvsClient, ShowVolumeDetailList));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询磁盘列表信息
 * 前置条件： 调用EVS获取失败，response为状态码非200
 * CHECK点：  调用查询磁盘列表信息失败
 */
TEST_F(HcsResourceAccessTest, GetVolumeResourceListFailedOtherStatus)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    Stub stub;
    stub.set(ADDR(EvsClient, ShowVolumeDetailList), StubGetVolumeListDetialFailedOtherStatus);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVolumeResourceList(page, parentId);
    stub.reset(ADDR(EvsClient, ShowVolumeDetailList));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询磁盘列表信息
 * 前置条件： 调用EVS获取磁盘信息成功，转换磁盘信息部分时部分失效
 * CHECK点：  调用查询磁盘列表信息成功
 */
TEST_F(HcsResourceAccessTest, GetVolumeResourceListFailedWhenComposeFailed)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    Stub stub;
    stub.set(ADDR(EvsClient, ShowVolumeDetailList), StubGetVolumeListDetialSuccess);
    ResourceResultByPage page;
    std::string parentId;
    int ret = hcsResourceAccess.GetVolumeResourceList(page, parentId);
    stub.reset(ADDR(EvsClient, ShowVolumeDetailList));
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例： 查询主机详情成功
 * 前置条件： 调用hcs接口成功
 * CHECK点： 1.GetVirtualMachineList返回值为SUCCESS;2.主机列表大小为1
 */
TEST_F(HcsResourceAccessTest, GetHostDetailSuccess)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    ResourceResultByPage returnValue;
    std::vector<std::string> vmLists = {"17af2355-5865-43a8-af30-59c82c9d49e5"};
    Stub stub;
    stub.set(ADDR(EcsClient, GetServerDetails), Stub_Get_ServerDetailsSuccess);
    stub.set(ADDR(EvsClient, ShowVolumeDetail), Stub_Get_VolumeDetailsSuccess);
    int ret = hcsResourceAccess.GetVirtualMachineDetailInfo(returnValue, vmLists);
    stub.reset(ADDR(EcsClient, GetServerDetails));
    stub.reset(ADDR(EvsClient, ShowVolumeDetail));
    EXPECT_EQ(returnValue.items.size(), 1);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 测试用例： 查询主机详情失败
 * 前置条件： 调用hcs接口，查询主机信息失败
 * CHECK点： GetVirtualMachineList返回值为FAILED
 */
TEST_F(HcsResourceAccessTest, GetHostDetailFailed_when_GetSeverDetail)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    ResourceResultByPage returnValue;
    std::vector<std::string> vmLists = {"17af2355-5865-43a8-af30-59c82c9d49e5"};
    Stub stub;
    stub.set(ADDR(EcsClient, GetServerDetails), Stub_Get_ServerDetailsFailed);
    int ret = hcsResourceAccess.GetVirtualMachineDetailInfo(returnValue, vmLists);
    stub.reset(ADDR(EcsClient, GetServerDetails));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询主机详情失败
 * 前置条件： 调用hcs接口，查询磁盘信息失败
 * CHECK点： GetVirtualMachineList返回值为FAILED
 */
TEST_F(HcsResourceAccessTest, GetHostDetailFailed_when_GetVolDetail)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    ResourceResultByPage returnValue;
    std::vector<std::string> vmLists = {"17af2355-5865-43a8-af30-59c82c9d49e5"};
    Stub stub;
    stub.set(ADDR(EcsClient, GetServerDetails), Stub_Get_ServerDetailsSuccess);
    stub.set(ADDR(EvsClient, ShowVolumeDetail), Stub_Get_VolumeDetailsFailed);
    int ret = hcsResourceAccess.GetVirtualMachineDetailInfo(returnValue, vmLists);
    stub.reset(ADDR(EcsClient, GetServerDetails));
    stub.reset(ADDR(EvsClient, ShowVolumeDetail));
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询主机列表失败
 * 前置条件： 打桩pm下发参数错误，缺少project
 * CHECK点： GetVirtualMachineList返回值为FAILED
 */
TEST_F(HcsResourceAccessTest, GetHostDetailFailed_taskParamError)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamError();
    ResourceResultByPage returnValue;
    std::vector<std::string> vmLists = {"17af2355-5865-43a8-af30-59c82c9d49e5"};
    int ret = hcsResourceAccess.GetVirtualMachineDetailInfo(returnValue, vmLists);
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 查询主机列表失败
 * 前置条件： Disk的Shareable为ture时
 * CHECK点： GetVirtualMachineList返回值为SUCCESS
 */
TEST_F(HcsResourceAccessTest, GetHostDetailFailedShareableTrue)
{
    HcsResourceAccess hcsResourceAccess = GetTestInstanceWhenTaskParamOK();
    ResourceResultByPage returnValue;
    std::vector<std::string> vmLists = {"17af2355-5865-43a8-af30-59c82c9d49e5"};
    Stub stub;
    stub.set(ADDR(EcsClient, GetServerDetails), Stub_Get_ServerDetailsSuccess);
    stub.set(ADDR(HcsResourceAccess, GetSpcifiedEVSDisk), StubGetSpcifiedEVSDiskOK);
    stub.set(ADDR(EvsClient, ShowVolumeDetail), Stub_Get_VolumeDetailsSuccess);
    int ret = hcsResourceAccess.GetVirtualMachineDetailInfo(returnValue, vmLists);
    stub.reset(ADDR(EcsClient, GetServerDetails));
    stub.reset(ADDR(HcsResourceAccess, GetSpcifiedEVSDisk));
    stub.reset(ADDR(EvsClient, ShowVolumeDetail));
    EXPECT_EQ(ret, SUCCESS);
}

int32_t Stub_ExecTaskSuccess()
{
    return SUCCESS;
}

bool Stub_ExecTaskTrue()
{
    return true;
}

/*
 * 测试用例： 检查存储连通性成功
 * 前置条件： 下发参数正确，同时检查连通性成功
 * CHECK点： CheckStorageConnect成功
 */
TEST_F(HcsResourceAccessTest, DiscoverAppClusterParmSuc)
{
    Authentication envAuth;
    envAuth.__set_extendInfo(g_mutiHcsStorageAuthExtendInfo);
    Stub stub;
    stub.set(ADDR(DistributedStorageMgr, CheckDistributedConnection), Stub_ExecTaskSuccess);
    stub.set(ADDR(StorageMgr, GetStorageSystemInfo), Stub_ExecTaskTrue);

    ApplicationEnvironment appEnv = m_appEnv;
    appEnv.__set_auth(envAuth);
    HcsResourceAccess hcsResourceAccess(appEnv);
    ApplicationEnvironment returnEnv;
    int iRet = hcsResourceAccess.CheckStorageConnect(returnEnv);
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 检查存储连通性失败
 * 前置条件： PM下发参数缺少校验存储设备的信息
 * CHECK点： CheckStorageConnect失败
 */
TEST_F(HcsResourceAccessTest, DiscoverAppClusterParmErr)
{
    Authentication envAuth;
    envAuth.__set_extendInfo("{\"enableCert\" : 0, \"certification\" : \"\"}");

    ApplicationEnvironment appEnv = m_appEnv;
    appEnv.__set_auth(envAuth);
    HcsResourceAccess hcsResourceAccess(appEnv);
    ApplicationEnvironment returnEnv;
    int iRet = hcsResourceAccess.CheckStorageConnect(returnEnv);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： PM下发环境参数解析失败
 * CHECK点： CheckAppConnect
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectParmFailed)
{
    m_appInfo.__set_extendInfo("{\"certName\" : \"\",\"certSize\":\"\",\"crlName\":\"\",\"crlSize\": \"\",\"domain\" : \"mo_bss_admin\",\"enableCert\":\"0\",\"ip\":\"88.5.1.3\",\"storages\":\"[{\"username\":\"admin\",\"port\":8088,\"ip\":\"88.1.100.100\",\"enableCert\":\"NaN\"}]\"}");
    HcsResourceAccess hcsResourceAccess(m_appEnv, m_appInfo);
    ActionResult actionResult;
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： PM下发环境数据不含domain参数
 * CHECK点： CheckAppConnect
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectParmFailedNoDomain)
{
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    hcsResourceAccess.m_application.__set_extendInfo("{\"certName\" : \"\",\"certSize\":\"\",\"crlName\":\"\",\"crlSize\": \"\",\"enableCert\":\"0\",\"ip\":\"88.5.1.3\",\"storages\":[{\"username\":\"admin\",\"port\":8088,\"ip\":\"88.1.100.100\",\"enableCert\":\"NaN\"}]}");
    ActionResult actionResult;
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： 检查VDC失败，VDC列表为空
 * CHECK点： CheckAppConnect，失败
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectCheckVdcFailedListEmpty)
{
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    ActionResult actionResult;
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： 检查VDC失败，序列化失败
 * CHECK点： CheckAppConnect，失败
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectCheckFailed)
{
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");
    appAuth.__set_extendInfo("{\"vdcInfos\" : \"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei\\\"}]\"}");
    hcsResourceAccess.m_application.__set_auth(appAuth);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    ActionResult actionResult;
    Stub stub;
    stub.set(ADDR(ScClient, QueryVdcList), StubQueryVdcListSuccess);
    stub.set(ADDR(QueryVdcListResponse, Serial), StubSerialFailed);
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
    stub.reset(ADDR(ScClient, QueryVdcList));
    stub.reset(ADDR(QueryVdcListResponse, Serial));
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： 检查VDC失败，VDC名不一致
 * CHECK点： CheckAppConnect，失败
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectCheckVdcFaileName)
{
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    Authentication appAuth;
    appAuth.__set_authkey("adminSS");
    appAuth.__set_authPwd("passwd");
    appAuth.__set_extendInfo("{\"vdcInfos\" : \"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei\\\"}]\"}");
    hcsResourceAccess.m_application.__set_auth(appAuth);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    ActionResult actionResult;
    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVdcsInfoUnderTenantByScApi), StubGetVdcsInfoUnderTenantByScApiSuccess);
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
    stub.reset(ADDR(HcsResourceAccess, GetVdcsInfoUnderTenantByScApi));
}

/*
 * 测试用例： 检查环境连通性
 * 前置条件： 检查VDC成功，获取token失败
 * CHECK点： CheckAppConnect，失败
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectCheckVdcFailedToken)
{
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");
    appAuth.__set_extendInfo("{\"vdcInfos\" : \"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei\\\"}]\"}");
    hcsResourceAccess.m_application.__set_auth(appAuth);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    ActionResult actionResult;
    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVdcsInfoUnderTenantByScApi), StubGetVdcsInfoUnderTenantByScApiSuccess);
    stub.set(ADDR(IamClient, GetToken), IamStubGetTokenFailed);
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, FAILED);
    stub.reset(ADDR(HcsResourceAccess, GetVdcsInfoUnderTenantByScApi));
    stub.reset(ADDR(IamClient, GetToken));
}
std::string HCSStub_ReadUserRole(std::string section, std::string keyName)
{
    return "vdcServiceManager";
}
/*
 * 测试用例： 检查环境连通性
 * 前置条件： 检查连通性成功
 * CHECK点： CheckAppConnect
 */
TEST_F(HcsResourceAccessTest, CheckAppConnectCheckSuccess)
{
    m_appEnv.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    HcsResourceAccess hcsResourceAccess(m_appEnv);
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");
    appAuth.__set_extendInfo("{\"vdcInfos\" : \"[{\\\"name\\\":\\\"huangrong\\\",\\\"passwd\\\":\\\"Huawei\\\"}]\"}");
    hcsResourceAccess.m_application.__set_auth(appAuth);
    hcsResourceAccess.m_application.__set_extendInfo("{\"domain\" :\"huangrong\",\"domainId\":\"99076361b95f4226b18db0001555bd00\", \"envName\":\"huangrong\", \"envPasswd\":\"Huawei!@#$1234\",\"isVdc\":\"true\"}");
    ActionResult actionResult;
    Stub stub;

    stub.set(ADDR(HttpClient, Send), Stub_CheckAppConnect);
    stub.set(ADDR(Module::ConfigReader, getString), HCSStub_ReadUserRole);
    int iRet = hcsResourceAccess.CheckAppConnect(actionResult);
    EXPECT_EQ(iRet, SUCCESS);
    stub.reset(ADDR(HttpClient, Send));
}

/*
 * 测试用例： 检查存储连通性失败
 * 前置条件： 获取系统信息正确
 * CHECK点： CheckStorageConnect
 */
TEST_F(HcsResourceAccessTest, DiscoverAppClusterGetSystemInfoTrue)
{
    Authentication envAuth;
    envAuth.__set_extendInfo("{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\",\"storages\":\"[{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"Admin@1234\\\",\\\"port\\\":8088,\\\"ip\\\":\\\"88.1.100.100\\\",\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":\\\"\\\",\\\"revocationlist\\\":\\\"\\\",\\\"parent\\\":null}]\"}");
    ApplicationEnvironment appEnv = m_appEnv;
    appEnv.__set_auth(envAuth);
    HcsResourceAccess hcsResourceAccess(appEnv);
    ApplicationEnvironment returnEnv;
    Stub stub;
    stub.set(ADDR(StorageMgr, GetStorageSystemInfo), StubGetStorageSystemInfoTrue);
    std::string storageIp;
    int iRet = hcsResourceAccess.CheckStorageConnect(returnEnv);
    EXPECT_EQ(iRet, SUCCESS);
    stub.reset(ADDR(StorageMgr, GetStorageSystemInfo));
}

/*
 * 测试用例： 检查存储连通性失败
 * 前置条件： 获取系统信息不正确
 * CHECK点： CheckStorageConnect
 */
TEST_F(HcsResourceAccessTest, DiscoverAppClusterGetSystemInfoFalse)
{
    Authentication envAuth;
    envAuth.__set_extendInfo("{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\",\"storages\":\"[{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"Admin@1234\\\",\\\"port\\\":8088,\\\"ip\\\":\\\"88.1.100.100\\\",\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":\\\"\\\",\\\"revocationlist\\\":\\\"\\\",\\\"parent\\\":null}]\"}");
    ApplicationEnvironment appEnv = m_appEnv;
    appEnv.__set_auth(envAuth);
    HcsResourceAccess hcsResourceAccess(appEnv);
    ApplicationEnvironment returnEnv;
    Stub stub;
    stub.set(ADDR(StorageMgr, GetStorageSystemInfo), StubGetStorageSystemInfoFalse);
    std::string storageIp;
    int iRet = hcsResourceAccess.CheckStorageConnect(returnEnv);
    EXPECT_EQ(iRet, FAILED);
    stub.reset(ADDR(StorageMgr, GetStorageSystemInfo));
}

}

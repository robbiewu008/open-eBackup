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
#include <sys/socket.h> 
#include "curl_http/HttpClientInterface.h"
#include <protect_engines/hcs/HCSProtectEngine.h>
#include "common/client/RestClient.h"
#include <protect_engines/hcs/api/ecs/EcsClient.h>
#include <protect_engines/hcs/common/HcsCommonInfo.h>
#include <protect_engines/hcs/common/HcsConstants.h>
#include <common/model/ModelBase.h>
#include <common/model/ResponseModel.h>
#include <common/httpclient/HttpClient.h>
#include "common/CommonMock.h"
#include <protect_engines/hcs/utils/HCSTokenMgr.h>
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "protect_engines/hcs/utils/IHttpClientMock.h"
#include "protect_engines/hcs/resource_discovery/HcsResourceAccess.h"
#include "protect_engines/hcs/utils/IHttpResponseMock.h"

using namespace OpenStackPlugin;
using HcsPlugin::HcsCinderClient;


namespace HDT_TEST {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using namespace HcsPlugin;
using namespace std;

const std::string t_volExtendInfo =
    "{\"targetVolumes\":\"[{\\\"uuid\\\":\\\"53f52bff-8ffa-46ae-98b4-a1f20aa99a7a\\\"},\
    {\\\"uuid\\\":\\\"2856e31e-242a-42e7-a992-713cd5bdc691\\\"}]\"}";
int32_t g_sendCount = 0;

class HCSProtectEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void InitLogger();

public:
    Stub stub;
private:
    AppProtect::BackupJob m_backupInfo;
    AppProtect::RestoreJob m_restoreInfo;
};


void HCSProtectEngineTest ::SetUp()
{
    InitLogger();
    stub.set(sleep, Stub_Sleep);
    g_sendCount = 0;
    // 设置恢复参数
    m_restoreInfo.requestId = "123";
    m_restoreInfo.jobId = "123";
    m_restoreInfo.targetEnv.__set_id("136");
    m_restoreInfo.targetEnv.__set_name("HcsPlanet");
    m_restoreInfo.targetEnv.__set_type("Virtual");
    m_restoreInfo.targetEnv.__set_endpoint("demo.com");
    m_restoreInfo.targetEnv.auth.__set_authkey("bss_admin");
    m_restoreInfo.targetEnv.auth.__set_authPwd("xxxxxxxx");
    m_restoreInfo.targetEnv.auth.__set_extendInfo(
        "{\"vdcInfo\":\"{\\\"name\\\":\\\"huangrong\\\", \\\"passwd\\\":\\\"Huawei\\\","
        "\\\"domainName\\\":\\\"sc-sh\\\"}\",\"enableCert\":\"0\",\"certification\":\"\",\"storages\":\"\"}");
    m_restoreInfo.targetEnv.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");
    // 卷参数
    ApplicationResource restoreVol;
    restoreVol.type = "volume";
    restoreVol.id = "1-1-1-1";                                                  // 源卷
    restoreVol.__set_extendInfo(t_volExtendInfo);  // 目标卷信息
    m_restoreInfo.restoreSubObjects = {restoreVol};
    m_restoreInfo.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    m_restoreInfo.__set_extendInfo("{\"restoreLevel\":\"1\"}");
    AppProtect::Copy copyInfo;
    AppProtect::StorageRepository repo;
    copyInfo.repositories.push_back(repo);
    m_restoreInfo.copies.push_back(copyInfo);

    // 设置备份参数
    m_backupInfo.requestId = "123";
    m_backupInfo.jobId = "123";
    m_backupInfo.protectEnv.__set_id("136");
    m_backupInfo.protectEnv.__set_name("HcsPlanet");
    m_backupInfo.protectEnv.__set_type("Virtual");
    m_backupInfo.protectEnv.__set_endpoint("demo.com");
    m_backupInfo.protectEnv.auth.__set_authkey("bss_admin");
    m_backupInfo.protectEnv.auth.__set_authPwd("xxxxxxxx");
    m_backupInfo.protectEnv.auth.__set_extendInfo(
        "{\"vdcInfo\":\"{\\\"name\\\":\\\"huangrong\\\", \\\"passwd\\\":\\\"Huawei\\\","
        "\\\"domainName\\\":\\\"sc-sh\\\"}\",\"enableCert\":\"0\",\"certification\":\"\",\"storages\":\"\"}");
    m_backupInfo.protectEnv.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");

}

void HCSProtectEngineTest ::TearDown()
{}

void HCSProtectEngineTest ::SetUpTestCase()
{}

void HCSProtectEngineTest ::TearDownTestCase()
{}

void HCSProtectEngineTest::InitLogger()
{
    std::string logFileName = "virt_plugin_hcs_engine_test.log";
    std::string logFilePath = "/tmp/log/";
    int logLevel = DEBUG;
    int logFileCount = 10;
    int logFileSize = 30;
    Module::CLogger::GetInstance().Init(
        logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
}

const std::string g_vmInfoStr = "{\
    \"extendInfo\": \"\",\
    \"metaData\": \"\",\
    \"moRef\": \"c69a9760-cb4d-44b1-b2fe-d5ab6ba19a88\",\
    \"targetFolderLocation\": \"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\
    \"volList\": [{\
        \"datastore\": {\
            \"dcMoRef\": \"\",\
            \"extendInfo\": \"{\\\"volId\\\":\\\"323\\\",\\\"volName\\\":\\\"baolong-volume-3ac7-scsi\\\",\\\"volWwn\\\":\\\"658f987100b749bc2d53e46c00000143\\\"}\",\
            \"ip\": \"\",\
            \"moRef\": \"2102351NPT10J3000001\",\
            \"name\": \"\",\
            \"poolId\": \"\",\
            \"port\": \"\",\
            \"type\": \"OceanStorV5\"\
        },\
        \"extendInfo\": \"\",\
        \"metadata\": \"\",\
        \"moRef\": \"256\",\
        \"name\": \"volume-ec68\",\
        \"slotId\": \"\",\
        \"type\": \"normal\",\
        \"uuid\": \"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\
        \"vmMoRef\": \"c69a9760-cb4d-44b1-b2fe-d5ab6ba19a88\",\
        \"volSizeInBytes\": 10737418240\
    }]\
}";

const std::string g_snapDetailStr = "{\"snapshot\":{\"status\":\"available\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\", \"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_snapDetailStrCreating = "{\"snapshot\":{\"status\":\"creating\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_snapDetailStrDeleting = "{\"snapshot\":{\"status\":\"deleting\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_snapDetailStrError = "{\"snapshot\":{\"status\":\"error\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";

const std::string g_snapListStr = 
    "{\"snapshots\":[{\"status\":\"available\",\"metadata\":{\"name\":\"test1\"},\"os-extended-snapshot-attributes:"
    "progress\":\"100%\",\"name\":\"Protect_129_SNAP_1660809407\",\"volume_id\":\"173f7b48-c4c1-4e70-9acc-086b39073506\","
    "\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\","
    "\"os-extended-snapshot-attributes:project_id\":\"bab7d5c60cd041a0a36f7c4b6e1dd978\",\"created_at\":\"2015-11-"
    "29T02:25:51.000000\",\"updated_at\":\"2015-11-29T02:26:28.000000\",\"size\":1,\"id\":\"b1323cda-8e4b-41c1-afc5-"
    "2fc791809c8c\",\"description\":\"volume "
    "snapshot\"},{\"status\":\"available\",\"metadata\":{\"name\":\"test2\"},\"os-extended-snapshot-attributes:"
    "progress\":\"100%\",\"name\":\"test-volume-snap\",\"volume_id\":\"173f7b48-c4c1-4e70-9acc-086b39073506\","
    "\"provider_auth\": \"{\\\"lun_id\\\": \\\"6605\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\","
    "\"os-extended-snapshot-attributes:project_id\":\"bab7d5c60cd041a0a36f7c4b6e1dd978\",\"created_at\":\"2015-11-"
    "29T02:27:51.000000\",\"updated_at\":\"2015-11-29T02:28:28.000000\",\"size\":1,\"id\":\"b1323cda-8e4b-41c1-afc5-"
    "2fc791809c8d\",\"description\":\"volume snapshot\"}]}";

const std::string g_snapListStr2 = 
    "{\"snapshots\":[{\"status\":\"available\",\"metadata\":{\"name\":\"test1\"},\"os-extended-snapshot-attributes:"
    "progress\":\"100%\",\"name\":\"test_129_SNAP_1660809407\",\"volume_id\":\"173f7b48-c4c1-4e70-9acc-086b39073506\","
    "\"provider_auth\": \"{\\\"lun_id\\\": \\\"6606\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\","
    "\"os-extended-snapshot-attributes:project_id\":\"bab7d5c60cd041a0a36f7c4b6e1dd978\",\"created_at\":\"2015-11-"
    "29T02:25:51.000000\",\"updated_at\":\"2015-11-29T02:26:28.000000\",\"size\":1,\"id\":\"b1323cda-8e4b-41c1-afc5-"
    "2fc791809c8c\",\"description\":\"volume snapshot\"}]}";

const std::string t_serverDetail =
    "{\"server\":{\"tenant_id\":\"e38d227edcce4631be20bfa5aad7130b\",\"addresses\":{\"subnet-8f61\":[{\"OS-EXT-IPS-MAC:"
    "mac_addr\":\"fa:16:3e:0b:7a:38\",\"OS-EXT-IPS:type\":\"fixed\",\"addr\":\"192.168.0.216\",\"version\":4}]},"
    "\"metadata\":{\"productId\":\"5b4ecaa32947446b824df4a6c60c8a04\",\"__instance_vwatchdog\":\"false\",\"_ha_policy_"
    "type\":\"remote_rebuild\",\"server_expiry\":\"0\",\"cascaded.instance_extrainfo\":\"max_cpu:254,current_mem:8192,"
    "org_mem:8192,iohang_timeout:720,pcibridge:2,system_serial_number:10a4e361-c981-46f2-b9ba-d7ff9c601693,max_mem:"
    "4194304,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:True,current_cpu:4,uefi_mode_sysinfo_fields:"
    "version_serial_uuid_family_asset,num_of_mem_plug:57\"},\"OS-EXT-STS:task_state\":null,\"OS-DCF:diskConfig\":"
    "\"MANUAL\",\"OS-EXT-AZ:availability_zone\":\"az0.dc0\",\"links\":[{\"rel\":\"self\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"},{\"rel\":\"bookmark\",\"href\":\"https://ecs.sc-cd-1.demo.com/"
    "e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"}],\"OS-EXT-STS:power_state\":4,\"id\":\"10a4e361-c981-46f2-b9ba-"
    "d7ff9c601693\",\"os-extended-volumes:volumes_attached\":[{\"id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\"}],\"OS-"
    "EXT-SRV-ATTR:host\":\"EA918B93-2561-E611-9B2A-049FCAD22DFC\",\"image\":{\"links\":[{\"rel\":\"bookmark\",\"href\":"
    "\"https://ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/images/"
    "a0a24ff5-1899-4c38-843a-659fd9d3ac15\"}],\"id\":\"a0a24ff5-1899-4c38-843a-659fd9d3ac15\"},\"OS-SRV-USG:terminated_"
    "at\":null,\"accessIPv4\":\"\",\"accessIPv6\":\"\",\"created\":\"2022-05-11T03:00:26Z\",\"hostId\":"
    "\"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\"OS-EXT-SRV-ATTR:hypervisor_hostname\":\"EA918B93-"
    "2561-E611-9B2A-049FCAD22DFC\",\"key_name\":null,\"flavor\":{\"links\":[{\"rel\":\"bookmark\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/flavors/"
    "ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"}],\"id\":\"ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"},\"security_groups\":[{"
    "\"name\":\"default\"}],\"config_drive\":\"\",\"OS-EXT-STS:vm_state\":\"stopped\",\"OS-EXT-SRV-ATTR:instance_"
    "name\":\"instance-00000030\",\"user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"name\":\"ecs-4d45-0001\",\"OS-SRV-"
    "USG:launched_at\":\"2022-05-11T03:00:36.000000\",\"updated\":\"2022-06-28T09:54:06Z\",\"status\":\"SHUTOFF\"}}";

const std::string t_volDetails =
    "{\"volume\":{\"id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\"status\":\"in-use\",\"attachments\":[{\"server_"
    "id\":\"10a4e361-c981-46f2-b9ba-d7ff9c601693\",\"attachment_id\":\"5aa3989e-acd7-467d-a436-9afdeb59865b\",\"host_"
    "name\":null,\"volume_id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\"device\":\"/dev/"
    "vda\",\"id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\"}],\"size\":10,\"volume_image_metadata\":{\"architecture\":"
    "\"x86_64\"},\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"129\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\"}}";

std::string g_check_app_token = "tokenStr";

std::string g_check_app_tokenBody =
    "{\"token\":{\"expires_at\":\"2022-07-22T07:59:59.869000Z\",\"methods\":[\"password\"],\"catalog\":[],\"roles\":[{"
    "\"name\":\"vdc_adm\",\"id\":\"ca71e771bafc42999098f088a784c751\"},{\"name\":\"tag_adm\",\"id\":"
    "\"d6bb5dbf74b44b95aaf2e70bfc72514f\"}],\"project\":{\"domain\":{\"name\":\"huangrong\",\"id\":"
    "\"99076361b95f4226b18db0001555bd00\"},\"name\":\"sc-cd-1_test\",\"id\":\"e38d227edcce4631be20bfa5aad7130b\"},"
    "\"issued_at\":\"2022-07-21T07:59:59.869000Z\",\"user\":{\"domain\":{\"name\":\"huangrong\",\"id\":"
    "\"99076361b95f4226b18db0001555bd00\"},\"name\":\"huangrong\",\"id\":\"d4216b7d3ba64a4eb63db37c2b91222c\"}}}";
const std::string g_mutiStorageAuthExtendInfo =
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
const std::string g_cinderCertinfo =
    "{\"cpsCertification\" : \"-----BEGIN "
    "CERTIFICATE-----"
    "\r\nMIIEsTCCApmgAwIBAgIRdjl5z9FobnagzdStBIQZVIcwDQYJKoZIhvcNAQELBQAw\r\nPDELMAkGA1UEBhMCQ04xDzANBgNVBAoTBkh1YXdlaT"
    "EcMBoGA1UEAxMTSHVhd2Vp\r\nIEVxdWlwbWVudCBDQTAeFw0xNjEwMTgwNjUwNTNaFw00MTEwMTIwNjUwNTNaMD0x\r\nCzAJBgNVBAYTAkNOMQ8w"
    "DQYDVQQKEwZIdWF3ZWkxHTAbBgNVBAMTFEh1YXdlaSBJ\r\nVCBQcm9kdWN0IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtKE3\r"
    "\n0649koONgSJqzwKXpSxTwiGTGorzcd3paBGH75Zgm5GFv2K2TG3cU6seS6dt7Ig+\r\n/"
    "8ntrcieQUttcWxpm2a1IBeohU1OTGFpomQCRqesDnlXXUS4JgZiDvPBzoqGCZkX\r\nYRw37J5KM5TSZzdLcWgxAPjXvKPdLXfxGzhqg8GV1tTboqX"
    "oNEqVqOeViBjsjN7i\r\nxIuu1Stauy9E0E5ZnSrwUjHc5QrR9CmWIu9D0ZJJp1M9VgcXy9evPhiHoz9o+KBd\r\nfNwt4e/NymTqaPa+ngS/"
    "qZwI7A4tR4RKCMKFHJcsjaXwUb0RuIeCiPO3wPHgXmGL\r\nuiKfyPV8SMLpE/"
    "wYaQIDAQABo4GsMIGpMB8GA1UdIwQYMBaAFCr4EFkngDUfp3y6\r\nO58q5Eqqm5LqMEYGA1UdIAQ/"
    "MD0wOwYEVR0gADAzMDEGCCsGAQUFBwIBFiVodHRw\r\nOi8vc3VwcG9ydC5odWF3ZWkuY29tL3N1cHBvcnQvcGtpMA8GA1UdEwQIMAYBAf8C\r\nAQ"
    "AwDgYDVR0PAQH/"
    "BAQDAgEGMB0GA1UdDgQWBBQSijfs+XNX1+SDurVvA+"
    "zdrhFO\r\nzzANBgkqhkiG9w0BAQsFAAOCAgEAAg1oBG8YFvDEecVbhkxU95svvlTKlrb4l77u\r\ncnCNhbnSlk8FVc5CpV0Q7SMeBNJhmUOA2xdF"
    "sfe0eHx9P3Bjy+difkpID/ow7oBH\r\nq2TXePxydo+AxA0OgAvdgF1RBPTpqDOF1M87eUpJ/"
    "DyhiBEE5m+QZ6VqOi2WCEL7\r\nqPGRbwjAFF1SFHTJMcxldwF6Q/"
    "QWUPMm8LUzod7gZrgP8FhwhDOtGHY5nEhWdADa\r\nF9xKejqyDCLEyfzsBKT8V4MsdAo6cxyCEmwiQH8sMTLerwyXo2o9w9J7+"
    "vRAFr2i\r\ntA7TwGF77Y1uV3aMj7n81UrXxqx0P8qwb467u+"
    "3Rj2Cs29PzhxYZxYsuov9YeTrv\r\nGfG9voXz48q8ELf7UOGrhG9e0yfph5UjS0P6ksbYInPXuuvrbrDkQvLBYb9hY78a\r\npwHn89PhRWE9HQwN"
    "nflTZS1gWtn5dQ4uvWAfX19e87AcHzp3vL4J2bCxxPXEE081\r\n3vhqtnU9Rlv/EJAMauZ3DKsMMsYX8i35ENhfto0ZLz1Aln0qtUOZ63h/"
    "VxQwGVC0\r\nOCE1U776UUKZosfTmNLld4miJnwsk8AmLaMxWOyRsqzESHa2x1t2sXF8s0/"
    "LW5T7\r\nd+j7JrLzey3bncx7wceASUUL3iAzICHYr728fNzXKV6OcZpjGdYdVREpM26sbxLo\r\n77rH32o=\r\n-----END "
    "CERTIFICATE-----\r\n\", \"cpsRevocationList\" : \"\", \"enableCert\" : \"1\"}";

const std::string t_tokenStr = "ASIFGHQOIAFVGBHOASDIVHBNAOFH9Q0FHQOIFHCNASKIS";
const std::string t_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
const std::string t_volumeId = "d5fb423e-9bd1-429a-8441-91efdef2b5f1";
const std::string t_projectId = "e38d227edcce4631be20bfa5aad7130b";
const std::string t_endpoint = "https://ecs.sc-cd-1.demo.com/v2";
const std::string t_domain = "test";

const std::string g_central_storage_type = "0";
const std::string g_distributed_storage_type = "1";

const std::string g_iSCSIMode = "ISCSI";
const std::string g_VBSMode = "VBS";

const std::string g_extendInfoWithStorage = R"({\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\",\\\"storageType\\\": \\\"0\\\",}]\"})";

bool Stub_GetTokenSuccess(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = t_tokenStr;
    endPoint = t_endpoint;
    return true;
}

bool Stub_HcsGetTokenFaild(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    return false;
}

int32_t Stub_CheckStorageEnvConnSuccess(const ApplicationEnvironment &appEnv, std::string &storageIp){
    return SUCCESS;
}

int32_t Stub_CheckStorageEnvConnFailed(const ApplicationEnvironment &appEnv, std::string &storageIp){
    return FAILED;
}

int32_t Stub_SendRequestSuccess(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody("");
    return SUCCESS;
}

int32_t Stub_SendRequestFailed(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    return FAILED;
}

int32_t HCSStub_UnlockServerRequestSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::ACCEPTED));
    return SUCCESS;
}

int32_t HCSStub_UnlockServerRequestFail(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::BAD_REQUEST));
    return SUCCESS;
}

int32_t Stub_SendRequestToGetServerDetailSuccess(
    void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(t_serverDetail);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    return SUCCESS;
}

void GetVolDetailsWhenStatusAvailable(std::string &volDetailStr)
{
    HSCVolDetail volDetail;
    Module::JsonHelper::JsonStringToStruct(t_volDetails, volDetail);
    volDetail.m_hostVolume.m_status = "available";
    std::vector<Attachments> attachments;
    volDetail.m_hostVolume.m_attachPoints = attachments;
    Module::JsonHelper::StructToJsonString(volDetail, volDetailStr);
}

void GetVolDetailsWhenOtherServer(std::string &volDetailStr)
{
    HSCVolDetail volDetail;
    Module::JsonHelper::JsonStringToStruct(t_volDetails, volDetail);
    for (auto &attachment : volDetail.m_hostVolume.m_attachPoints) {
        attachment.m_serverId = "other";
    }
    Module::JsonHelper::StructToJsonString(volDetail, volDetailStr);
}

int32_t Stub_SendRequestToGetVolDetailSuccessWhenOtherServer(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    std::string volDetailStr;
    GetVolDetailsWhenOtherServer(volDetailStr);
    response->SetSuccess(true);
    response->SetGetBody(volDetailStr);
    return SUCCESS;
}

int32_t Stub_SendRequestToGetVolDetailSuccessWhenAvailable(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    std::string volDetailStr;
    GetVolDetailsWhenStatusAvailable(volDetailStr);
    response->SetSuccess(true);
    response->SetGetBody(volDetailStr);
    return SUCCESS;
}

int32_t Stub_SendRequestToGetVolDetailSuccessWhenInuse(
    void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(t_volDetails);
    return SUCCESS;
}

int32_t Stub_SendRequestToDetachVolumeSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(t_volDetails);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody("");
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 2) {
        std::string volDetail = t_volDetails;
        int index = volDetail.find("in-use");
        std::string status = "in-use";
        volDetail.replace(index, status.size(), SNAPSHOT_STATUS_AVALIABLE);
        response->SetSuccess(true);
        response->SetGetBody(volDetail);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
}

int32_t Stub_SendRequestToAttachVolumeSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        std::string volDetailStr;
        GetVolDetailsWhenStatusAvailable(volDetailStr);
        response->SetSuccess(true);
        response->SetGetBody(volDetailStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody("");
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 2) {
        response->SetSuccess(true);
        response->SetGetBody(t_volDetails);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
}

void GetServerDetailsWhenStatusActive(std::string &serverDetailStr)
{
    ServerDetail serverDetail;
    Module::JsonHelper::JsonStringToStruct(t_serverDetail, serverDetail);
    serverDetail.m_hostServerInfo.m_status = "ACTIVE";
    Module::JsonHelper::StructToJsonString(serverDetail, serverDetailStr);
}

int32_t Stub_SendRequestToGetServerDetailSuccessWhenActive(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    std::string serverDetailStr;
    GetServerDetailsWhenStatusActive(serverDetailStr);
    response->SetSuccess(true);
    response->SetGetBody(serverDetailStr);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
    return SUCCESS;
}

int32_t Stub_SendRequestToPowerOffServerSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        std::string serverDetailStr;
        GetServerDetailsWhenStatusActive(serverDetailStr);
        response->SetSuccess(true);
        response->SetGetBody(serverDetailStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody("");
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::ACCEPTED));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 2) {
        response->SetSuccess(true);
        response->SetGetBody(t_serverDetail);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
}

int32_t Stub_SendRequestToPowerOnServerSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(t_serverDetail);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody("");
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::ACCEPTED));
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 2) {
        std::string serverDetailStr;
        GetServerDetailsWhenStatusActive(serverDetailStr);
        response->SetSuccess(true);
        response->SetGetBody(serverDetailStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        g_sendCount++;
        return SUCCESS;
    }
}

static int count = 0;

int32_t Stub_SendRequest(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    if (count == 0) {
        Stub_SendRequestToGetServerDetailSuccess(obj, request, response);
        count = 1;
    } else {
        Stub_SendRequestToGetVolDetailSuccessWhenInuse(obj, request, response);
    }
    return SUCCESS;
}

int32_t Stub_ConfigReaderSnapshotResult(std::string section, std::string keyName)
{
    return 10;
}

/**
 * 测试用例：成功解定虚拟机
 * 前置条件：锁定虚拟机成功
 * Check点：PostHook返回SUCCESS
 */
TEST_F(HCSProtectEngineTest, PreHook_lockVM_SUCC)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_UnlockServerRequestSucc);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    int ret = hcsProtectEngineHandler->PreHook(param);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：PostHook锁定虚拟机失败
 * 前置条件：锁定虚拟机失败
 * Check点：PostHook返回FAILED
 */
TEST_F(HCSProtectEngineTest, PreHook_lockVM_FAIL)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_UnlockServerRequestFail);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    int ret = hcsProtectEngineHandler->PreHook(param);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：成功解锁定虚拟机
 * 前置条件：解锁定虚拟机成功
 * Check点：PreHook返回SUCCESS
 */
TEST_F(HCSProtectEngineTest, PreHook_UnlockVM_SUCC)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_UnlockServerRequestSucc);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    ExecHookParam param;
    param.stage = JobStage::PRE_PREREQUISITE;
    int ret = hcsProtectEngineHandler->PostHook(param);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：解锁虚拟机失败
 * 前置条件：发送消息解锁定虚拟机失败
 * Check点：PreHook返回FAILED
 */
TEST_F(HCSProtectEngineTest, PreHook_UnlockVM_FAIL)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_UnlockServerRequestFail);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    ExecHookParam param;
    param.stage = JobStage::PRE_PREREQUISITE;
    int ret = hcsProtectEngineHandler->PostHook(param);
    EXPECT_EQ(ret, FAILED);
}

std::string Stub_ConfigReaderBackupResult(std::string section, std::string keyName)
{
    return "active,stopped,suspended,in-use";
}

/**
 * 测试用例：CheckBeforeBackup接口测试
 * 前置条件：所有检查点均满足备份条件
 * Check点：CheckBeforeBackup返回SUCCESS
 */
TEST_F(HCSProtectEngineTest, CheckBeforeBackupSucc)
{
    count = 0;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequest);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderBackupResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：CheckBeforeBackup接口测试
 * 前置条件：检查存储连通性失败
 * Check点：CheckBeforeBackup返回FAILED
 */
TEST_F(HCSProtectEngineTest, CheckBeforeBackupFail)
{
    count = 0;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequest);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_HcsGetTokenFaild);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderBackupResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, FAILED);
}

std::string Stub_ConfigReaderResult(std::string section, std::string keyName)
{
    return "active,stopped,in-use";
}

/**
 * 测试用例：CheckBeforeRestore接口测试
 * 前置条件：所有检查点均满足恢复条件
 * Check点：CheckBeforeRestore返回SUCCESS
 */
TEST_F(HCSProtectEngineTest, CheckBeforeRestoreSucc)
{
    count = 0;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequest);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(HCSProtectEngine, CheckStorageEnvConn), Stub_CheckStorageEnvConnSuccess);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderResult);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo restoreVol;
    restoreVol.m_uuid = "1-1-1-1";
    restoreVol.m_volSizeInBytes = 10;
    restoreVol.m_metadata = t_volDetails;
    vmInfo.m_volList.push_back(restoreVol);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：卷大小不一致导致恢复前检查接口失败
 * 前置条件：副本卷大小与目标卷大小不一致
 * Check点：CheckBeforeRestore返回FAILED
 */
TEST_F(HCSProtectEngineTest, CheckBeforeRestoreFail_VolSizeNotMatch)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequest);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderResult);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo restoreVol;
    restoreVol.m_uuid = "1-1-1-1";
    restoreVol.m_volSizeInBytes = 9;
    restoreVol.m_metadata = t_volDetails;
    vmInfo.m_volList.push_back(restoreVol);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试PowerOffMachine接口成功
 * 前置条件：与平台通信正常
 * Check点：虚拟机已下电，直接返回成功
 */
TEST_F(HCSProtectEngineTest, PoweroffSuccessWhenServerShutOff)
{
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetServerDetailSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：测试PowerOffMachine接口成功
 * 前置条件：与平台通信正常
 * Check点：调用下电接口返回成功
 */
TEST_F(HCSProtectEngineTest, PoweroffSuccessWhenServerActive)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToPowerOffServerSuccess);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(g_sendCount, 3);
    g_sendCount = 0;
}

/**
 * 测试用例：测试PowerOffMachine接口失败
 * 前置条件：与平台通信正常
 * Check点：获取虚拟机详情接口失败，返回失败。
 */
TEST_F(HCSProtectEngineTest, PoweroffFailedWhenGetServerFailed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestFailed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试PowerOffMachine接口失败
 * 前置条件：与平台通信正常
 * Check点：虚拟机状态一直是ACTIVE，返回失败。
 */
TEST_F(HCSProtectEngineTest, PoweroffFailedWhenAlwaysActive)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetServerDetailSuccessWhenActive);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试PowerOnMachine接口成功
 * 前置条件：与平台通信正常
 * Check点：虚拟机已上电，直接返回成功
 */
TEST_F(HCSProtectEngineTest, PoweronSuccessWhenServerActive)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetServerDetailSuccessWhenActive);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：测试PowerOnMachine接口成功
 * 前置条件：与平台通信正常
 * Check点：调用上电接口返回成功。
 */
TEST_F(HCSProtectEngineTest, PoweronSuccessWhenServerShutOff)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToPowerOnServerSuccess);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(g_sendCount, 3);
    g_sendCount = 0;
}

/**
 * 测试用例：测试PowerOnMachine接口失败
 * 前置条件：与平台通信正常
 * Check点：获取虚拟机详情接口失败，返回失败。
 */
TEST_F(HCSProtectEngineTest, PoweronFailedWhenGetServerFailed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestFailed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试PowerOnMachine接口失败
 * 前置条件：与平台通信正常
 * Check点：虚拟机状态一直是SHUTOFF，返回失败。
 */
TEST_F(HCSProtectEngineTest, PoweronFailedWhenAlwaysShutoff)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetServerDetailSuccess);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试DetachVolume接口成功
 * 前置条件：与平台通信正常，参数正确
 * Check点：返回成功。
 */
TEST_F(HCSProtectEngineTest, DetachVolumeSuccessWhenVolumeAttached)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToDetachVolumeSuccess);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    VolAttachMents volAttachMents;
    volAttachMents.m_device = "/dev/vda";
    volInfo.m_attachPoints.push_back(volAttachMents);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(g_sendCount, 3);
    g_sendCount = 0;
}

/**
 * 测试用例：测试DetachVolume接口成功
 * 前置条件：与平台通信正常，参数正确
 * Check点：卷已卸载，直接返回成功。
 */
TEST_F(HCSProtectEngineTest, DetachVolumeSuccessWhenVolumeDetached)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetVolDetailSuccessWhenAvailable);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：测试DetachVolume接口失败
 * 前置条件：与平台通信正常，参数正确
 * Check点：卷挂载在其他虚拟机上。
 */
TEST_F(HCSProtectEngineTest, DetachVolumeSuccessWhenVolumeAttachToOther)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetVolDetailSuccessWhenOtherServer);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：测试AttachVolume接口成功
 * 前置条件：与平台通信正常，参数正确
 * Check点：返回成功。
 */
TEST_F(HCSProtectEngineTest, AttachVolumeSuccessWhenVolumeDetach)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToAttachVolumeSuccess);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    VolAttachMents volAttachMents;
    volAttachMents.m_device = "/dev/vda";
    volInfo.m_attachPoints.push_back(volAttachMents);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(g_sendCount, 3);
    g_sendCount = 0;
}

/**
 * 测试用例：测试AttachVolume接口成功
 * 前置条件：与平台通信正常，参数正确
 * Check点：卷已挂载，直接返回成功。
 */
TEST_F(HCSProtectEngineTest, AttachVolumeFailedWhenVolumeAttachedToOtherServer)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetVolDetailSuccessWhenInuse);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：测试AttachVolume接口失败
 * 前置条件：与平台通信正常，参数正确
 * Check点：卷已挂载，但没有挂载在指定虚拟机上，返回失败。
 */
TEST_F(HCSProtectEngineTest, AttachVolumeSuccessWhenVolumeAttached)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetVolDetailSuccessWhenOtherServer);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    VMInfo vmInfo;
    vmInfo.m_uuid = t_serverId;
    VolInfo volInfo;
    volInfo.m_uuid = t_volumeId;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    int ret = hcsProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：生成卷匹配对关系成功
 * 前置条件：查询卷信息成功
 * Check点：GenVolPair调用返回SUCCESS
 */
TEST_F(HCSProtectEngineTest , GenVolPairSucc)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetVolDetailSuccessWhenInuse);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcsProtectEngineHandler = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcsProtectEngineHandler->InitJobPara();
    VMInfo vmObj;
    VolInfo copyVol;
    VolMatchPairInfo volPairs;
    ApplicationResource targetVol;
    targetVol.extendInfo = t_volExtendInfo;
    int ret = hcsProtectEngineHandler->GenVolPair(vmObj, copyVol, targetVol, volPairs);
    EXPECT_EQ(ret, SUCCESS);
}

static Module::IHttpClient *Stub_check_application_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(201));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_check_app_tokenBody));
    std::set<std::string> headerValue;
    headerValue.insert(g_check_app_token);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* Stub_check_application_Failed()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(400));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_check_app_tokenBody));
    std::set<std::string> headerValue;
    headerValue.insert(g_check_app_token);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}
struct hostent* gethostbyname_Failed(const char* domain)
{
    struct hostent* host;
    host->h_addr_list[0][0] = 88;
    host->h_addr_list[0][1] = 1;
    host->h_addr_list[0][2] = 1;
    host->h_addr_list[0][3] = 18;
    return host;
}

TEST_F(HCSProtectEngineTest, CheckApplicationSuccess)
{
    /*app*/
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");

    Application appInfo;
    appInfo.__set_type("type");
    appInfo.__set_subType("subType");
    appInfo.__set_id("id");
    appInfo.__set_name("name");
    appInfo.__set_parentId("parentId");
    appInfo.__set_parentName("parentName");
    appInfo.__set_auth(appAuth);
    appInfo.__set_extendInfo("{\"domain\":\"domain\", \"ip\":\"12:1:1:1\"}");

    /*env*/
    Authentication envAuth;
    envAuth.__set_authkey("admin");
    envAuth.__set_authPwd("passwd");
    envAuth.__set_extendInfo("{\"enableCert\":\"0\",\"certification\":\"\",\"storages\":\"\"}");

    ApplicationEnvironment appEnv;
    appEnv.__set_id("envId");
    appEnv.__set_name("envName");
    appEnv.__set_type("envType");
    appEnv.__set_subType("envSubType");
    appEnv.__set_endpoint("demo.com");
    appEnv.__set_port(8088);
    appEnv.__set_auth(envAuth);
    std::vector<ApplicationEnvironment> nodes= {};
    appEnv.__set_nodes(nodes);
    appEnv.__set_extendInfo("");

    ActionResult returnValue;
    returnValue.__set_code(-1);
    HCSProtectEngine hcsProtectEngine;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_check_application_Success);
    stub.set(gethostbyname, gethostbyname_Failed);
    hcsProtectEngine.CheckApplication(returnValue, appEnv, appInfo);
    EXPECT_EQ(returnValue.code, 1577210003);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(gethostbyname);
}

TEST_F(HCSProtectEngineTest, CheckApplicationFailed)
{
    /*app*/
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");

    Application appInfo;
    appInfo.__set_type("type");
    appInfo.__set_subType("subType");
    appInfo.__set_id("id");
    appInfo.__set_name("name");
    appInfo.__set_parentId("parentId");
    appInfo.__set_parentName("parentName");
    appInfo.__set_auth(appAuth);
    appInfo.__set_extendInfo("{\"domain\":\"domain\",\"ip\":\"88.1.1.18\"}");

    /*env*/
    Authentication envAuth;
    envAuth.__set_authkey("admin");
    envAuth.__set_authPwd("passwd");

    ApplicationEnvironment appEnv;
    appEnv.__set_id("envId");
    appEnv.__set_name("envName");
    appEnv.__set_type("envType");
    appEnv.__set_subType("envSubType");
    appEnv.__set_endpoint("demo.com");
    appEnv.__set_port(8088);
    appEnv.__set_auth(envAuth);
    std::vector<ApplicationEnvironment> nodes= {};
    appEnv.__set_nodes(nodes);
    appEnv.__set_extendInfo("");

    ActionResult returnValue;
    returnValue.__set_code(-1);
    HCSProtectEngine hcsProtectEngine;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_check_application_Failed);
    stub.set(gethostbyname, gethostbyname_Failed);
    hcsProtectEngine.CheckApplication(returnValue, appEnv, appInfo);
    EXPECT_EQ(returnValue.code, 1677930259); // 修改后上报的错误码
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(gethostbyname);
}

int32_t GetTenantInfo_Failed(ResourceResultByPage& page)
{
    return FAILED;
}
int32_t GetProjectLists_Failed(ResourceResultByPage& page, std::map<int, std::vector<std::string>>& errorCodeAndName)
{
    return FAILED;
}

int32_t GetVDCListsPartialSuccess(HcsResourceAccess* obj, ResourceResultByPage &page,
    std::vector<std::string>& errorUserName, std::string &parentId)
{
    ApplicationResource appResource;
    appResource.__set_type("VDC");
    appResource.__set_subType("HCSVDC");
    appResource.__set_id("");
    appResource.__set_name("");
    appResource.__set_parentId("test");
    appResource.__set_parentName("test");
    appResource.__set_extendInfo("");
    page.items.push_back(appResource);
    errorUserName.push_back("test_01");

    return SUCCESS;
}

int32_t GetProjectListsVDCDeleteFailed(HcsResourceAccess* obj, ResourceResultByPage &page,
    std::vector<std::string>& errorCodeAndName, std::string &parentId)
{
    errorCodeAndName.push_back("test_01");
    return FAILED;
}

int32_t GetVirtualMachineListFailed(HcsResourceAccess* obj, ResourceResultByPage& page)
{
    return FAILED;
}
int32_t GetVirtualMachineDetailInfo_Failed(ResourceResultByPage &returnValue, const std::vector<std::string> &vmLists)
{
    return FAILED;
}

int32_t GetTenantInfo_Success(ResourceResultByPage& page)
{
    return SUCCESS;
}
int32_t GetProjectLists_Success(ResourceResultByPage& page, std::map<int, std::vector<std::string>>& errorCodeAndName)
{
    return SUCCESS;
}
int32_t GetVirtualMachineListSuccess(HcsResourceAccess* obj, ResourceResultByPage& page)
{
    return SUCCESS;
}
int32_t GetVirtualMachineDetailInfo_Success(ResourceResultByPage &returnValue, const std::vector<std::string> &vmLists)
{
    return SUCCESS;
}


static int32_t GetVolumeResourceListSuccess(HcsResourceAccess* obj, ResourceResultByPage& page)
{
    return SUCCESS;
}

static int32_t GetVolumeResourceListFailed(HcsResourceAccess* obj, ResourceResultByPage& page)
{
    return FAILED;
}

int32_t Stub_CheckSuccess(ApplicationEnvironment &returnEnv)
{
    return SUCCESS;
}

TEST_F(HCSProtectEngineTest, ListApplicationResourceV2Failed)
{
    ListResourceRequest request;

    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "tenant";
    stub.set(ADDR(HcsResourceAccess, GetTenantInfo), GetTenantInfo_Failed);

    request.condition.conditions = "project";
    stub.set(ADDR(HcsResourceAccess, GetProjectLists), GetProjectLists_Failed);

    request.condition.conditions = "vm";
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineList), GetVirtualMachineListFailed);
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineDetailInfo), GetVirtualMachineDetailInfo_Failed);
    stub.reset(ADDR(HcsResourceAccess, GetTenantInfo));
    stub.reset(ADDR(HcsResourceAccess, GetProjectLists));
    stub.reset(ADDR(HcsResourceAccess, GetVirtualMachineList));
    stub.reset(ADDR(HcsResourceAccess, GetVirtualMachineDetailInfo));
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);

    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    page.__set_total(1);
    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 1);
}

TEST_F(HCSProtectEngineTest, ListApplicationResourceV2Success)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "tenant";
    stub.set(ADDR(HcsResourceAccess, GetTenantInfo), GetTenantInfo_Success);
    request.condition.conditions = "project";
    stub.set(ADDR(HcsResourceAccess, GetProjectLists), GetProjectLists_Success);

    request.condition.conditions = "vm";
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineList), GetVirtualMachineListSuccess);
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineDetailInfo), GetVirtualMachineDetailInfo_Success);
    stub.reset(ADDR(HcsResourceAccess, GetTenantInfo));
    stub.reset(ADDR(HcsResourceAccess, GetProjectLists));
    stub.reset(ADDR(HcsResourceAccess, GetVirtualMachineList));
    stub.reset(ADDR(HcsResourceAccess, GetVirtualMachineDetailInfo));
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);

    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    page.__set_total(1);
    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 1);
}

/*
 * 测试用例：获取租户信息
 * 前置条件：获取租户列表信息成功
 * CHECK点：获取租户信息成功
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2SuccessTenantInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"tenant\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetTenantInfo), GetTenantInfo_Success);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    page.__set_total(1);

    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 1);
}

/*
 * 测试用例：获取租户信息
 * 前置条件：获取租户列表信息失败
 * CHECK点：获取租户信息失败
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2FailedTenantInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"tenant\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetTenantInfo), GetTenantInfo_Failed);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;

    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 0);
}

/*
 * 测试用例：获取虚拟机信息
 * 前置条件：获取虚拟机列表信息成功
 * CHECK点：获取虚拟机信息成功
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2SuccessVMInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"vm\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineList), GetVirtualMachineListSuccess);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    page.__set_total(1);

    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 1);
}

/*
 * 测试用例：获取虚拟机信息
 * 前置条件：获取虚拟机列表信息失败
 * CHECK点：获取虚拟机信息失败
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2FailedVMInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"vm\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVirtualMachineList), GetVirtualMachineListFailed);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 0);
}

/*
 * 测试用例：获取磁盘信息
 * 前置条件：获取磁盘列表信息成功
 * CHECK点：获取虚拟机信息成功
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2SuccessVolumeInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"disk\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVolumeResourceList), GetVolumeResourceListSuccess);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    page.__set_total(1);
    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 1);
}

/*
 * 测试用例：获取虚拟机信息
 * 前置条件：获取虚拟机列表信息失败
 * CHECK点：获取虚拟机信息失败
 */
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2FailedVolumeInfo)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"disk\"}";

    Stub stub;
    stub.set(ADDR(HcsResourceAccess, GetVolumeResourceList), GetVolumeResourceListFailed);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    hcsProtectEngine.ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.total, 0);
}

// VDC列表部分删除，任务部分成功
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2PartialSuccess)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"vdc\"}";
    stub.set(ADDR(HcsResourceAccess, GetVDCResourceList), GetVDCListsPartialSuccess);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    hcsProtectEngine.ListApplicationResourceV2(page, request);

    EXPECT_EQ(page.items.size(), 2);
}

// VDC管理员删除，任务失败
TEST_F(HCSProtectEngineTest, ListApplicationResourceV2VDCDeleteFailed)
{
    ListResourceRequest request;
    Application application;
    application.__set_id("id");
    std::vector<Application> appVec;
    appVec.push_back(application);
    request.__set_applications(appVec);
    request.condition.conditions = "{\"sourceType\":\"project\"}";
    stub.set(ADDR(HcsResourceAccess, GetVDCResourceList), GetProjectListsVDCDeleteFailed);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_CheckSuccess);
    HCSProtectEngine hcsProtectEngine;
    ResourceResultByPage page;
    hcsProtectEngine.ListApplicationResourceV2(page, request);

    EXPECT_EQ(page.items.size(), 1);
}

std::string authExtendInfo = "{\"enableCert\" : 0, \"certification\" : \"\",\"storages\" : \"[{\"username\":\"admin\",\"password\":\"Admin@1234\",\"ip\":\"8.40.99.81\",\"port\":8088,\"enableCert\":0,\"certification\":\"\"}]\"}";

TEST_F(HCSProtectEngineTest, DiscoverAppClusterSuccess)
{
    /*app*/
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");

    Application appInfo;
    appInfo.__set_type("type");
    appInfo.__set_subType("subType");
    appInfo.__set_id("id");
    appInfo.__set_name("name");
    appInfo.__set_parentId("parentId");
    appInfo.__set_parentName("parentName");
    appInfo.__set_auth(appAuth);
    appInfo.__set_extendInfo("{\"dommian\":\"domain\"}");

    /*env*/
    Authentication envAuth;
    envAuth.__set_authkey("admin");
    envAuth.__set_authPwd("passwd");
    envAuth.__set_extendInfo(authExtendInfo);

    ApplicationEnvironment appEnv;
    appEnv.__set_id("envId");
    appEnv.__set_name("envName");
    appEnv.__set_type("envType");
    appEnv.__set_subType("envSubType");
    appEnv.__set_endpoint("demo.com");
    appEnv.__set_port(8088);
    appEnv.__set_auth(envAuth);
    std::vector<ApplicationEnvironment> nodes= {};
    appEnv.__set_nodes(nodes);
    appEnv.__set_extendInfo("");

    ApplicationEnvironment returnValue;
    returnValue.__set_name("name");
    HCSProtectEngine hcsProtectEngine;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_check_application_Success);
    hcsProtectEngine.DiscoverAppCluster(returnValue, appEnv, appInfo);
    EXPECT_EQ(returnValue.name, "name");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

TEST_F(HCSProtectEngineTest, DiscoverAppClusterFailed)
{
    /*app*/
    Authentication appAuth;
    appAuth.__set_authkey("admin");
    appAuth.__set_authPwd("passwd");

    Application appInfo;
    appInfo.__set_type("type");
    appInfo.__set_subType("subType");
    appInfo.__set_id("id");
    appInfo.__set_name("name");
    appInfo.__set_parentId("parentId");
    appInfo.__set_parentName("parentName");
    appInfo.__set_auth(appAuth);
    appInfo.__set_extendInfo("{\"dommian\":\"domain\"}");

    /*env*/
    Authentication envAuth;
    envAuth.__set_authkey("admin");
    envAuth.__set_authPwd("passwd");
    envAuth.__set_extendInfo(authExtendInfo);

    ApplicationEnvironment appEnv;
    appEnv.__set_id("envId");
    appEnv.__set_name("envName");
    appEnv.__set_type("envType");
    appEnv.__set_subType("envSubType");
    appEnv.__set_endpoint("demo.com");
    appEnv.__set_port(8088);
    appEnv.__set_auth(envAuth);
    std::vector<ApplicationEnvironment> nodes= {};
    appEnv.__set_nodes(nodes);
    appEnv.__set_extendInfo("");

    ApplicationEnvironment returnValue;
    returnValue.__set_name("name");
    HCSProtectEngine hcsProtectEngine;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_check_application_Failed);
    hcsProtectEngine.DiscoverAppCluster(returnValue, appEnv, appInfo);
    EXPECT_EQ(returnValue.name, "name");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

static int32_t Stub_SendRequestTo_CreateSnapshot_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_snapDetailStr);
    response->SetStatusCode(202);
    return SUCCESS;
}

static std::shared_ptr<GetSnapshotResponse> StubShowSnapshotSuccess(GetSnapshotRequest &request)
{
    std::shared_ptr<GetSnapshotResponse> response = std::make_shared<GetSnapshotResponse>();
    response->SetSuccess(true);
    response->SetGetBody(g_snapDetailStr);
    response->SetStatusCode(200);
    return response;
}

static std::string StubGetProviderAuthSuccess(GetSnapshotResponse* obj)
{
    return "\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}";
}

static std::shared_ptr<ActiveSnapConsistencyResponse> StubActiveSnapConsistencySuccess(ActiveSnapConsistencyRequest &request)
{
    std::shared_ptr<ActiveSnapConsistencyResponse> response = std::make_shared<ActiveSnapConsistencyResponse>();
    response->SetSuccess(true);
    response->SetGetBody("");
    response->SetStatusCode(200);
    return response;
}

static std::shared_ptr<ActiveSnapConsistencyResponse> StubActiveSnapConsistencyFailed(
    HcsCinderClient* obj, ActiveSnapConsistencyRequest &request)
{
    std::shared_ptr<ActiveSnapConsistencyResponse> response = nullptr;
    return response;
}

static std::shared_ptr<ActiveSnapConsistencyResponse> StubActiveSnapConsistencyFailed404(
    HcsCinderClient* obj, ActiveSnapConsistencyRequest &request)
{
    std::shared_ptr<ActiveSnapConsistencyResponse> response = std::make_shared<ActiveSnapConsistencyResponse>();
    response->SetSuccess(true);
    response->SetGetBody("");
    response->SetStatusCode(404);
    return response;
}

static int32_t Stub_SendRequestTo_CreateSnapshot_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(404);
    return FAILED;
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == 1) {
        response->SetSuccess(true);
        std::string body = "{\"itemNotFound\":{\"message\":\"Snapshot 30e15ed1-5ab5-4049-97a3-f7001b0707c5 could not be found.\",\"code\":404}}";
        response->SetGetBody(body);
        response->SetStatusCode(404);
        g_sendCount++;
        return SUCCESS;
    }
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_Success_When_No_Snapshot(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(404);
    return SUCCESS;
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(500);
    return FAILED;
}

static int32_t Stub_SendRequestTo_QuerySnapshot_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_snapDetailStr);
    return SUCCESS;
}

static int32_t Stub_SendRequestTo_QuerySnapshot_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(404);
    return FAILED;
}

static bool StubInitJobParaFailed()
{
    return false;
}

/*
 * 测试用例：创建快照成功
 * 前置条件：创建快照cinder接口返回成功
 * CHECK点：创建快照成功
 */
TEST_F(HCSProtectEngineTest, CreateSnapshot_Success)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_ConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(hcs->CreateSnapshot(snapshot, errCode), SUCCESS);
    EXPECT_EQ(snapshot.m_volSnapList[0].m_volUuid, hcs->m_vmInfo.m_volList[0].m_uuid);
}

/*
 * 测试用例：创建快照失败
 * 前置条件：创建快照cinder接口返回失败
 * CHECK点：创建快照失败
 */
TEST_F(HCSProtectEngineTest, CreateSnapshot_Failed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_ConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(hcs->CreateSnapshot(snapshot, errCode), FAILED);
}

static int32_t Stub_SendRequestTo_CreateSnapshot_WhenStatusError(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) {
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(g_snapDetailStrCreating);
        g_sendCount++;
        return SUCCESS;
    } else if (g_sendCount == 1) {
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(g_snapDetailStrError);
        g_sendCount++;
        return SUCCESS;
    } else if (g_sendCount == 2) {
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return SUCCESS;
    } else if (g_sendCount == 3) {
        response->SetSuccess(true);
        response->SetStatusCode(404);
        response->SetGetBody("{\"itemNotFound\":{\"message\":\"Snapshot 30e15ed1-5ab5-4049-97a3-f7001b0707c5 could not be found.\",\"code\":404}}");
        g_sendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例：创建快照失败
 * 前置条件：创建快照，快照状态为error
 * CHECK点：创建快照失败
 */
TEST_F(HCSProtectEngineTest, CreateSnapshot_WhenSnapStatusError)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_WhenStatusError);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_ConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(hcs->CreateSnapshot(snapshot, errCode), FAILED);
    g_sendCount = 0;
}

/*
 * 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回成功
 * CHECK点：删除快照成功
 */
TEST_F(HCSProtectEngineTest, DeleteSnapshot_Success)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->DeleteSnapshot(snapshot), SUCCESS);
    g_sendCount = 0;
}

/*
 * 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回成功, 快照不存在
 * CHECK点：删除快照成功
 */
TEST_F(HCSProtectEngineTest, DeleteSnapshot_Success_When_No_Snapshot)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_Success_When_No_Snapshot);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);
 
    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->DeleteSnapshot(snapshot), SUCCESS);
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_When_Status_Deleting(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) { // 第一次删除快照，接口返回202
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount >=1 && g_sendCount <=SNAPSHOT_RETRY_TIMES) { // 查询快照的状态一直不为已删除
        response->SetSuccess(true);
        response->SetGetBody(g_snapDetailStr);
        response->SetStatusCode(200);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+1) { // 查询快照状态为deleting
        response->SetSuccess(true);
        response->SetGetBody(g_snapDetailStrDeleting);
        response->SetStatusCode(200);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+2) { // 重置快照状态为error
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+3) { // 查询快照状态为error
        response->SetSuccess(true);
        response->SetGetBody(g_snapDetailStrError);
        response->SetStatusCode(200);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+4) { // 删除快照
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+5) { // 快照不存在
        response->SetSuccess(true);
        response->SetStatusCode(404);
        response->SetGetBody("{\"itemNotFound\":{\"message\":\"Snapshot 30e15ed1-5ab5-4049-97a3-f7001b0707c5 could not be found.\",\"code\":404}}");
        g_sendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回失败，快照状态重置成功，删除成功
 * CHECK点：删除快照成功
 */
TEST_F(HCSProtectEngineTest, DeleteSnapshot_Success_When_Status_Deleting)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_When_Status_Deleting);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->DeleteSnapshot(snapshot), SUCCESS);
    g_sendCount = 0;
}

/*
 * 测试用例：删除快照失败
 * 前置条件：删除快照cinder接口返回失败
 * CHECK点：删除快照失败
 */
TEST_F(HCSProtectEngineTest, DeleteSnapshot_Failed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->DeleteSnapshot(snapshot), FAILED);
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_When_Reset_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_sendCount == 0) { // 第一次删除快照，接口返回202
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_sendCount++;
        return VirtPlugin::SUCCESS;
    }
    if (g_sendCount >=1 && g_sendCount <=SNAPSHOT_RETRY_TIMES) { // 查询快照的状态一直不为已删除
        response->SetSuccess(true);
        response->SetGetBody(g_snapDetailStr);
        response->SetStatusCode(200);
        g_sendCount++;
        return VirtPlugin::SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+1) { // 查询快照状态为deleting
        response->SetSuccess(true);
        response->SetGetBody(g_snapDetailStrDeleting);
        response->SetStatusCode(200);
        g_sendCount++;
        return VirtPlugin::SUCCESS;
    }
    if (g_sendCount == SNAPSHOT_RETRY_TIMES+2) { // 重置快照状态为error
        response->SetSuccess(true);
        response->SetStatusCode(400);
        g_sendCount++;
        return VirtPlugin::SUCCESS;
    }
}

/*
 * 测试用例：删除快照失败
 * 前置条件：删除快照cinder接口返回失败，快照状态重置失败
 * CHECK点：删除快照失败
 */
TEST_F(HCSProtectEngineTest, DeleteSnapshot_Failed_WhenResetFailed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_When_Reset_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->DeleteSnapshot(snapshot), VirtPlugin::FAILED);
    g_sendCount = 0;
}

/*
 * 测试用例：查询快照是否存在
 * 前置条件：快照存在
 * CHECK点：查询快照存在
 */
TEST_F(HCSProtectEngineTest, QuerySnapshot_Success)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshot_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->QuerySnapshotExists(snapshot), SUCCESS);
}

/*
 * 测试用例：查询快照是否存在
 * 前置条件：快照不存在
 * CHECK点：查询快照不存在
 */
TEST_F(HCSProtectEngineTest, QuerySnapshot_Failed)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshot_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_vmInfoStr, hcs->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(hcs->QuerySnapshotExists(snapshot), FAILED);
}

static int32_t Stub_SendRequestTo_QuerySnapshotList_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_snapListStr);
    return SUCCESS;
}

static int32_t Stub_SendRequestTo_QuerySnapshotList2_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_snapListStr2);
    return SUCCESS;
}

static int32_t Stub_SendRequestTo_QuerySnapshotList_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(400);
    return SUCCESS;
}

/*
 * 测试用例：查询卷的所有之前创建的快照成功
 * 前置条件：卷、快照存在
 * CHECK点：查询成功
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotList_Success)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "FusionStorage";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), SUCCESS);
    EXPECT_EQ(snapList.size(), 1);
}

/*
 * 测试用例：查询卷的所有之前创建的快照成功
 * 前置条件：卷存在、快照不存在
 * CHECK点：查询成功，列表长度为0
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotList_Success_When_no_snap)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList2_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "FusionStorage";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), SUCCESS);
    EXPECT_EQ(snapList.size(), 0);
}

/*
 * 测试用例：查询卷的所有之前创建的快照失败
 * 前置条件：查询卷快照接口返回失败
 * CHECK点：查询失败
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotList_Failed_When_Return_400)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "FusionStorage";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), FAILED);
}

/*
 * 测试用例：查询卷的所有之前创建的快照成功,生产存储类型不匹配
 * 前置条件：卷、快照存在
 * CHECK点：查询失败
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotListSuccessWhenStorageTypeError)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "UnknowType";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), SUCCESS);
    EXPECT_EQ(snapList.size(), 0);
}

/*
 * 测试用例：查询卷的所有之前创建的快照成功,生产存储为dorado
 * 前置条件：卷、快照存在
 * CHECK点：查询成功
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotListSuccessWhenStorageIsDorado)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList_Success);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "DoradoV6";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), SUCCESS);
    EXPECT_EQ(snapList.size(), 1);
}

/*
 * 测试用例：查询卷的所有之前创建的快照失败,生产存储为dorado
 * 前置条件：查询卷快照接口返回失败
 * CHECK点：查询失败
 */
TEST_F(HCSProtectEngineTest, QuerySnapshotListFailedWhenStorageIsDorado)
{
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_QuerySnapshotList_Failed);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    VolInfo volInfo;
    volInfo.m_uuid = "173f7b48-c4c1-4e70-9acc-086b39073506";
    DatastoreInfo dataStore;
    dataStore.m_type = "DoradoV6";
    volInfo.m_datastore = dataStore;
    std::vector<VolSnapInfo> snapList;
    EXPECT_EQ(hcs->GetSnapshotsOfVolume(volInfo, snapList), FAILED);
}

int32_t HCSStub_SendHttpActiveSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // get server metadata
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(t_serverDetail);
        count += 1;
    } else if (count == 1) { // get volume info
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(t_volDetails);
        count += 1;
    } else if (count == 2) { // Get snap info
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(g_snapDetailStr);
        count += 1;
    } else {  // active snapshot
        response->SetSuccess(true);
        response->SetGetBody("");
        response->SetStatusCode(200);
    }
    return SUCCESS;
}

/*
 * 测试用例：快照一致性激活
 * 前置条件：激活快照一致性成功
 * CHECK点：接口返回成功
 */
TEST_F(HCSProtectEngineTest, ActiveSnapConsistencySuccess)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_SendHttpActiveSucc);
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    SnapshotInfo snapshotInfo;
    snapshotInfo.m_moRef = "123456789";
    snapshotInfo.m_vmName = "temp-vm1";
    snapshotInfo.m_vmMoRef = "111-222-333-444";
    VolSnapInfo volSnapinfo;
    volSnapinfo.m_snapshotId = "111111";
    snapshotInfo.m_volSnapList.push_back(volSnapinfo);

    VolInfo volInfo;
    volInfo.m_uuid = "123456789";
    volInfo.m_name = "temp_ll0";
    volInfo.m_volSizeInBytes = 10;
    hcs->m_vmInfo.m_volList.push_back(volInfo);
    EXPECT_EQ(hcs->ActiveSnapConsistency(snapshotInfo), SUCCESS);
    stub.reset(ADDR(HCSTokenMgr, GetToken));
    stub.reset(ADDR(HttpClient, Send));
}

int32_t HCSStub_SendHttpActiveFail_GetVMInfoFailed(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(404);
    return SUCCESS;
}

/*
 * 测试用例：快照一致性激活
 * 前置条件：获取虚拟机metadata失败
 * CHECK点：快照一致性激活失败
 */
TEST_F(HCSProtectEngineTest, ActiveSnapConsistencyMetaDataFailed)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_SendHttpActiveFail_GetVMInfoFailed);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    SnapshotInfo snapshotInfo;
    EXPECT_EQ(hcs->ActiveSnapConsistency(snapshotInfo), FAILED);
    stub.reset(ADDR(HttpClient, Send));
}

int32_t HCSStub_SendHttpActiveFail_ActiveFailed(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // get server metadata
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(t_serverDetail);
        count += 1;
    } else if (count == 1) { // get volume info
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(t_volDetails);
        count += 1;
    } else if (count == 2) { // Get snap info
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(g_snapDetailStr);
        count += 1;
    } else {  // active snapshot fail
        response->SetSuccess(false);
        response->SetGetBody("");
        response->SetStatusCode(401);
    }
    return SUCCESS;
}

/*
 * 测试用例：快照一致性激活
 * 前置条件：激活快照一致性失败
 * CHECK点：接口返回成功
 */
TEST_F(HCSProtectEngineTest, ActiveSnapConsistencyFailedNullptr)
{
    stub.set(ADDR(HttpClient, Send), HCSStub_SendHttpActiveFail_ActiveFailed);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_moRef = "123456789";
    snapshotInfo.m_vmName = "temp-vm1";
    snapshotInfo.m_vmMoRef = "111-222-333-444";
    VolSnapInfo volSnapinfo;
    volSnapinfo.m_snapshotId = "111111";
    snapshotInfo.m_volSnapList.push_back(volSnapinfo);

    VolInfo volInfo;
    volInfo.m_uuid = "123456789";
    volInfo.m_name = "temp_ll0";
    volInfo.m_volSizeInBytes = 10;
    hcs->m_vmInfo.m_volList.push_back(volInfo);
    EXPECT_EQ(hcs->ActiveSnapConsistency(snapshotInfo), FAILED);
    stub.reset(ADDR(HttpClient, Send));
}

int32_t Stub_GetBackupMode(std::string &backupMode){
    return SUCCESS;
}

int32_t Stub_GetStorageType(const ApplicationEnvironment &appEnv, std::string &storageType)
{
    return SUCCESS;
}

int32_t Stub_ResAccessCheckStorageConnect(ApplicationEnvironment &returnEnv)
{
    return SUCCESS;
}

int32_t Stub_UtilsGetAgentDeployScenceSuccess(std::string &deployScence)
{
    return SUCCESS;
}

int32_t Stub_UtilsGetAgentDeployScenceFailed(std::string &deployScence)
{
    return FAILED;
}

std::string Stub_ConfigReaderISCSIModeSuccess(const std::string &file, const std::string &param)
{
    return "ISCSI";
}

std::string Stub_ConfigReaderVBSModeSuccess(const std::string &file, const std::string &param)
{
    return "VBS";
}

bool Stub_ResAccessCheckStorageParseInfoSuccess(Json::Value &storagesJson)
{
    return true;
}

bool Stub_ResAccessCheckStorageParseInfoFailed(Json::Value &storagesJson)
{
    return false;
}

int32_t Stub_CheckProtectEnvConn(const AppProtect::ApplicationEnvironment& env, const std::string &vmId){
    return SUCCESS;
}

/*
 * 测试用例：检查备份任务类型
 * 前置条件：进行备份任务
 * CHECK点：检查任务类型，不是分布式存储且为iscsi模式，就不执行增量转全量
 */
TEST_F(HCSProtectEngineTest, CheckBackupJobTypeSuccess)
{
    Stub stub;
    stub.set(ADDR(Utils, GetAgentDeployScence), Stub_UtilsGetAgentDeployScenceSuccess);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderISCSIModeSuccess);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    // 新增检查存储增量转全量使用
    AppProtect::BackupJob g_backupInfo;
    g_backupInfo.protectEnv.auth.__set_extendInfo(g_mutiStorageAuthExtendInfo);
    g_backupInfo.protectEnv.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    g_backupInfo.protectObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    SnapshotInfo g_snapshotInfo;
    VirtPlugin::JobTypeParam jobTypeParam;
    jobTypeParam.m_job = g_backupInfo;
    jobTypeParam.m_snapshotInfo = g_snapshotInfo;
    VolSnapInfo volSnapInfoTest;
    volSnapInfoTest.m_datastore.m_ip = "88.1.1.22";
    volSnapInfoTest.m_datastore.m_moRef = "88.1.1.22";
    jobTypeParam.m_snapshotInfo.m_volSnapList.push_back(volSnapInfoTest);

    bool checkRet = false;
    int ret = hcs->CheckBackupJobType(jobTypeParam, checkRet);
    EXPECT_EQ(checkRet, true);
}

/*
 * 测试用例：检测和存储连通性
 * 前置条件：进行备份任务
 * CHECK点：检查任务类型，发现是外置Agent对接分布式存储且为iscsi模式
 */
TEST_F(HCSProtectEngineTest, CheckStorageEnvConnSuccess){
    Stub stub;
    stub.set(ADDR(HCSProtectEngine, GetHcsBackupMode), Stub_GetBackupMode);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_ResAccessCheckStorageConnect);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    ApplicationEnvironment appEnv;
    int32_t ret = hcs->CheckStorageEnvConn(appEnv);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 测试用例：分布式存储时获备份模式类型
 * 前置条件：进行备份任务，存储为分布式存储时，获取备份模式
 * CHECK点：获取备份模式成功
 */
TEST_F(HCSProtectEngineTest, GetHcsBackupModeSuccess){
    Stub stub;
    stub.set(ADDR(Utils, GetAgentDeployScence), Stub_UtilsGetAgentDeployScenceSuccess);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderISCSIModeSuccess);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    std::string backupMode;
    int32_t ret = hcs->GetHcsBackupMode(backupMode);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 测试用例：分布式存储时获备份模式类型
 * 前置条件：进行备份任务，存储为分布式存储时，获取备份模式
 * CHECK点：获取备份模式失败
 */
TEST_F(HCSProtectEngineTest, GetHcsBackupModeFailed){
    Stub stub;
    stub.set(ADDR(Utils, GetAgentDeployScence), Stub_UtilsGetAgentDeployScenceFailed);
    stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderVBSModeSuccess);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    std::string backupMode;
    int32_t ret = hcs->GetHcsBackupMode(backupMode);
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例：检测同存储和HCS资源连通性
 * 前置条件：进行备份任务
 * CHECK点：检查连通性成功
 */
TEST_F(HCSProtectEngineTest, CheckProtectEnvConnSuccess){
    Stub stub;
    stub.set(ADDR(HCSProtectEngine, GetHcsBackupMode), Stub_GetBackupMode);
    stub.set(ADDR(HcsResourceAccess, CheckStorageConnect), Stub_ResAccessCheckStorageConnect);
    stub.set(ADDR(HCSProtectEngine, CheckProtectEnvConn), Stub_CheckProtectEnvConn);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();
    ApplicationEnvironment appEnv;
    std::string vmId = "123";
    int32_t errorCode;

    int32_t ret = hcs->CheckEnvConnection(appEnv, vmId, errorCode);
    EXPECT_EQ(ret, SUCCESS);
}

static bool Stub_HCSSaveCertToFile(const std::string& fileName) {
    return true;
}

/*
 * 测试用例：解析cinder 证书
 * 前置条件：在下发的参数中存在cinder证书
 * CHECK点：解析cinder证书成功
 */
TEST_F(HCSProtectEngineTest, ParseCinderCertSuccess){
    Stub stub;
    stub.set(ADDR(CertManger, SaveCertToFile), Stub_HCSSaveCertToFile);
    
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<HCSProtectEngine> hcs = std::make_shared<HCSProtectEngine>(jobHandle, "123", "");
    hcs->InitJobPara();

    bool ret = hcs->ParseCinderCert(g_cinderCertinfo);
    EXPECT_EQ(ret, true);
}

}  // namespace HDT_TEST
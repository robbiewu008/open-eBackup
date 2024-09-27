package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * 功能描述 CnwareRestoreProviderTest
 *
 * @author q30048244
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-05 9:40
 */
@Slf4j
public class CnwareRestoreProviderTest {
    private static CnwareRestoreProvider restoreProvider;

    private static final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    @BeforeClass
    public static void init() {
        restoreProvider = new CnwareRestoreProvider(copyRestApi);
    }

    /**
     * 用例场景：Cnware恢复插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean cNwareVm = restoreProvider.applicable("CNwareVm");
        Assert.assertTrue(cNwareVm);
    }

    /**
     * 用例场景：Cnware恢复拦截补充正确 <br/>
     * 前置条件：Cnware恢复对象参数正确 <br/>
     * 检查点：返回恢复任务对象参数补充正确
     */
    @Test
    public void test_restore_intercept_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        RestoreTask interceptTask = restoreProvider.initialize(restoreTask);
        TaskResource targetObject = interceptTask.getTargetObject();
        Assert.assertEquals(CnwareConstant.POWER_ON, targetObject.getExtendInfo().get(CnwareConstant.POWER_STATE));
        Assert.assertEquals("/targetLocation", targetObject.getTargetLocation());
        Map<String, String> advanceParams = interceptTask.getAdvanceParams();
        Assert.assertEquals("0", advanceParams.get(CnwareConstant.RESTORE_LEVEL));
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTaskId(UUIDGenerator.getUUID());
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUIDGenerator.getUUID());
        protectObject.setName("TestName");
        protectObject.setParentUuid(UUIDGenerator.getUUID());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(CnwareConstant.POWER_STATE, CnwareConstant.POWER_ON);
        protectObject.setExtendInfo(extendInfo);
        restoreTask.setTargetObject(protectObject);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(CnwareConstant.RESTORE_LOCATION, "/targetLocation");
        advanceParams.put(CnwareConstant.RESTORE_LEVEL, "0");
        advanceParams.put(CnwareConstant.POWER_STATE, CnwareConstant.POWER_ON);
        restoreTask.setAdvanceParams(advanceParams);
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        restoreTask.setTargetEnv(taskEnvironment);
        TaskResource subObject = new TaskResource();
        subObject.setUuid("subObject_id");
        subObject.setName("subObject_name");
        restoreTask.setSubObjects(Collections.singletonList(subObject));
        return restoreTask;
    }

    /**
     * 用例场景：从磁带归档恢复时，Cnware恢复拦截补充正确 <br/>
     * 前置条件：Cnware恢复对象参数正确 <br/>
     * 检查点：返回恢复任务对象参数补充正确
     */
    @Test
    public void test_restore_intercept_success_when_copy_generated_by_tape_archive() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        RestoreTask interceptTask = restoreProvider.initialize(restoreTask);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), interceptTask.getRestoreMode());
    }

    private Copy mockCopy() {
        String copyJson = "{\"uuid\":\"2dea44eb-f4e7-46cf-9b83-2ceb5d5e1295\",\"chain_id\":\"290cd659-c2df-4ca7-9aba-0f61e0a67518\",\"timestamp\":\"1660571265000000\",\"display_timestamp\":\"2022-08-15T21:47:45\",\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":2,\"generated_by\":\"CloudArchive\",\"generated_time\":\"2022-08-15T21:47:45\",\"generation_type\":\"\",\"features\":2,\"indexed\":\"Unindexed\",\"generation\":1,\"parent_copy_uuid\":\"\",\"retention_type\":2,\"retention_duration\":3,\"duration_unit\":\"MO\",\"expiration_time\":1668548865000,\"properties\":\"{\\\"snapshots\\\":[{\\\"id\\\":\\\"69@2dea44eb-f4e7-46cf-9b83-2ceb5d5e1295\\\",\\\"parentName\\\":\\\"VM_4d8d0ba0-497e-575d-8d79-184be9b65808\\\"}],\\\"verifyStatus\\\":\\\"3\\\",\\\"repositories\\\":[{\\\"type\\\":1,\\\"protocol\\\":5,\\\"role\\\":0},{\\\"type\\\":2,\\\"protocol\\\":5,\\\"role\\\":0}],\\\"sub_object\\\":[],\\\"disk_info\\\":\\\"[{\\\\\\\"customProperties\\\\\\\":{},\\\\\\\"datastoreName\\\\\\\":\\\\\\\"FS\\\\\\\",\\\\\\\"datastoreUrn\\\\\\\":\\\\\\\"urn:sites:35B905D0:datastores:2\\\\\\\",\\\\\\\"drExtParams\\\\\\\":\\\\\\\"{\\\\\\\\\\\\\\\"dsMgntIp\\\\\\\\\\\\\\\":\\\\\\\\\\\\\\\"8.40.37.219\\\\\\\\\\\\\\\",\\\\\\\\\\\\\\\"dsResourceId\\\\\\\\\\\\\\\":\\\\\\\\\\\\\\\"0\\\\\\\\\\\\\\\"}\\\\\\\",\\\\\\\"indepDisk\\\\\\\":false,\\\\\\\"ioMode\\\\\\\":\\\\\\\"dataplane\\\\\\\",\\\\\\\"ioWeight\\\\\\\":0,\\\\\\\"isDiffVol\\\\\\\":false,\\\\\\\"isThin\\\\\\\":true,\\\\\\\"maxReadBytes\\\\\\\":0,\\\\\\\"maxReadRequest\\\\\\\":0,\\\\\\\"maxWriteBytes\\\\\\\":0,\\\\\\\"maxWriteRequest\\\\\\\":0,\\\\\\\"name\\\\\\\":\\\\\\\"i-0000001B-vda\\\\\\\",\\\\\\\"pciType\\\\\\\":\\\\\\\"VIRTIO\\\\\\\",\\\\\\\"persistentDisk\\\\\\\":true,\\\\\\\"pvscsiSupport\\\\\\\":0,\\\\\\\"quantityGB\\\\\\\":40,\\\\\\\"scsiCommandPassthrough\\\\\\\":false,\\\\\\\"sequenceNum\\\\\\\":1,\\\\\\\"siocFlag\\\\\\\":0,\\\\\\\"srcVolumeUrn\\\\\\\":\\\\\\\"null\\\\\\\",\\\\\\\"status\\\\\\\":\\\\\\\"USE\\\\\\\",\\\\\\\"storageType\\\\\\\":\\\\\\\"DSWARE\\\\\\\",\\\\\\\"storageVersion\\\\\\\":\\\\\\\"DSWARE\\\\\\\",\\\\\\\"systemVolume\\\\\\\":true,\\\\\\\"totalRWBytes\\\\\\\":0,\\\\\\\"totalRWRequest\\\\\\\":0,\\\\\\\"type\\\\\\\":\\\\\\\"normal\\\\\\\",\\\\\\\"uri\\\\\\\":\\\\\\\"/service/sites/35B905D0/volumes/82\\\\\\\",\\\\\\\"urn\\\\\\\":\\\\\\\"urn:sites:35B905D0:volumes:82\\\\\\\",\\\\\\\"userUsedSize\\\\\\\":5908,\\\\\\\"uuid\\\\\\\":\\\\\\\"03745467-dcc6-43ce-be58-5f1e674f55bc\\\\\\\",\\\\\\\"volInfoUrl\\\\\\\":\\\\\\\"fusionstorage://8.40.37.219/0/03745467-dcc6-43ce-be58-5f1e674f55bc\\\\\\\",\\\\\\\"volNameOnDev\\\\\\\":\\\\\\\"03745467-dcc6-43ce-be58-5f1e674f55bc\\\\\\\",\\\\\\\"volProvisionSize\\\\\\\":-1,\\\\\\\"volType\\\\\\\":0,\\\\\\\"volumeFormat\\\\\\\":\\\\\\\"raw\\\\\\\",\\\\\\\"volumeUrl\\\\\\\":\\\\\\\"03745467-dcc6-43ce-be58-5f1e674f55bc\\\\\\\",\\\\\\\"volumeUseType\\\\\\\":0}]\\\\n\\\",\\\"dataBeforeReduction\\\":4559282,\\\"dataAfterReduction\\\":1790016,\\\"multiFileSystem\\\":\\\"false\\\"}\",\"resource_id\":\"4d8d0ba0-497e-575d-8d79-184be9b65808\",\"resource_name\":\"FS-VM-001\",\"resource_type\":\"VM\",\"resource_sub_type\":\"FusionCompute\",\"resource_location\":\"8.40.98.196/ManagementCluster/FS-VM-001\",\"resource_status\":\"EXIST\",\"resource_environment_name\":\"fc\",\"resource_environment_ip\":\"8.40.98.196\",\"resource_properties\":\"{\\\"name\\\":\\\"FS-VM-001\\\",\\\"path\\\":\\\"8.40.98.196/ManagementCluster/FS-VM-001\\\",\\\"root_uuid\\\":\\\"ba3ede74-ab17-35bf-8066-fb1dc82e04b2\\\",\\\"parent_name\\\":\\\"ManagementCluster\\\",\\\"parent_uuid\\\":\\\"d9f56277-7442-5b17-b861-40385109c9e6\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"VM\\\",\\\"sub_type\\\":\\\"FusionCompute\\\",\\\"uuid\\\":\\\"4d8d0ba0-497e-575d-8d79-184be9b65808\\\",\\\"created_time\\\":\\\"2022-08-07T17:58:54.697000\\\",\\\"ext_parameters\\\":{\\\"pre_script\\\":null,\\\"post_script\\\":null,\\\"failed_script\\\":null,\\\"agents\\\":null,\\\"overwrite\\\":null,\\\"binding_policy\\\":null,\\\"resource_filters\\\":null,\\\"disk_filters\\\":null,\\\"disk_info\\\":[]},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"56defa38-52c1-4e65-9110-b29612c5973a\\\",\\\"sla_name\\\":\\\"Bronze\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":true,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"ba3ede74-ab17-35bf-8066-fb1dc82e04b2\\\",\\\"environment_name\\\":\\\"fc\\\",\\\"environment_endpoint\\\":\\\"8.40.98.196\\\",\\\"environment_os_type\\\":null,\\\"environment_type\\\":\\\"Platform\\\",\\\"environment_sub_type\\\":\\\"FusionCompute\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":null,\\\"extendInfo\\\":{\\\"vGroup\\\":\\\"vGeneral\\\",\\\"numberOfDisk\\\":\\\"999999999\\\",\\\"machineDes\\\":\\\"2150bd09-053b-46c2-80a8-02aa551290f0\\\",\\\"storageSize\\\":\\\"41943040\\\",\\\"objectGuid\\\":\\\"4d8d0ba0-497e-575d-8d79-184be9b65808\\\",\\\"hostId\\\":\\\"\\\",\\\"machineGuid\\\":\\\"2150bd09-053b-46c2-80a8-02aa551290f0\\\",\\\"machineOs\\\":\\\"EulerOS 2.8 64bit\\\",\\\"moReference\\\":\\\"/service/sites/35B905D0/vms/i-0000001B\\\"}}\",\"sla_name\":\"Bronze\",\"sla_properties\":\"{\\\"uuid\\\": \\\"56defa38-52c1-4e65-9110-b29612c5973a\\\", \\\"name\\\": \\\"Bronze\\\", \\\"created_time\\\": \\\"7777-01-09T20:21:32.842+08:00\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Common\\\", \\\"policy_list\\\": [{\\\"uuid\\\": \\\"7c011152-fb27-4326-baf6-3e71ffab0e82\\\", \\\"name\\\": \\\"full\\\", \\\"type\\\": \\\"backup\\\", \\\"action\\\": \\\"full\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 3, \\\"duration_unit\\\": \\\"MO\\\", \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 30, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2021-04-20T00:00:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\", \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"active\\\": true, \\\"is_active\\\": true}, {\\\"uuid\\\": \\\"19fb78cf-23b1-4388-99bd-aca6b2e4110a\\\", \\\"name\\\": \\\"difference_increment\\\", \\\"type\\\": \\\"backup\\\", \\\"action\\\": \\\"difference_increment\\\", \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"retention_duration\\\": 3, \\\"duration_unit\\\": \\\"MO\\\", \\\"daily_copies\\\": null, \\\"weekly_copies\\\": null, \\\"monthly_copies\\\": null, \\\"yearly_copies\\\": null}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 7, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2021-04-20T00:00:00\\\", \\\"window_start\\\": \\\"01:00:00\\\", \\\"window_end\\\": \\\"01:00:00\\\", \\\"days_of_week\\\": null, \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null}, \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5}, \\\"active\\\": true, \\\"is_active\\\": true}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null, \\\"is_global\\\": true}\",\"job_type\":\"\",\"user_id\":\"\",\"is_archived\":false,\"is_replicated\":false,\"name\":\"FS-VM-001_1660571265\",\"storage_id\":\"\",\"amount\":0,\"gn\":39,\"prev_copy_id\":\"7cee48eb-d51f-4641-81af-97e2c0c50fa9\",\"next_copy_id\":\"\",\"prev_copy_gn\":38,\"next_copy_gn\":0,\"deletable\":true}";
        return JSONObject.fromObject(copyJson).toBean(Copy.class);
    }

    private Copy mockReplicationCopy() {
        String repCopyJson = "{\"uuid\":\"ba31d86a-d41f-474f-be3f-e4c4320cee5d\",\"chain_id\":\"245af1ef-f79e-525e-8363-e54e0eab3175\",\"timestamp\":\"1663034537000000\",\"display_timestamp\":\"2022-09-13T10:17:33.513000\",\"deletable\":true,\"status\":\"Normal\",\"location\":\"m04\",\"backup_type\":2,\"generated_by\":\"CloudArchive\",\"generated_time\":\"2022-09-13T10:17:33.513000\",\"features\":2,\"indexed\":\"Unindexed\",\"generation\":1,\"parent_copy_uuid\":\"8569e52b-231c-4ff5-9349-18127acfcc63\",\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\",\"expiration_time\":\"2022-09-14T10:17:33.555000\",\"properties\":\"{\\\"snapshots\\\":[{\\\"id\\\":\\\"58@8569e52b-231c-4ff5-9349-18127acfcc63\\\",\\\"parentName\\\":\\\"Rep_247129_Huaage_VM_245af1ef-f79e-525e-8363-e54e0eab3175\\\"}],\\\"repositories\\\":[{\\\"id\\\":\\\"2102354PBB10MC100001\\\",\\\"type\\\":1,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":0,\\\"path\\\":\\\"/Rep_247129_Huaage_VM_245af1ef-f79e-525e-8363-e54e0eab3175/source_policy_245af1ef-f79e-525e-8363-e54e0eab3175_Context_Global_MD\\\"},{\\\"type\\\":1,\\\"path\\\":\\\"/Rep_247129_Huaage_VM_245af1ef-f79e-525e-8363-e54e0eab3175/source_policy_245af1ef-f79e-525e-8363-e54e0eab3175_Context\\\"}],\\\"extendInfo\\\":{\\\"esn\\\":\\\"2102354PBB10MC100001\\\"}},{\\\"id\\\":\\\"2102354PBB10MC100001\\\",\\\"type\\\":2,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":1,\\\"path\\\":\\\"/VM_CacheDataRepository/245af1ef-f79e-525e-8363-e54e0eab3175\\\"}],\\\"extendInfo\\\":{\\\"esn\\\":\\\"2102354PBB10MC100001\\\"}},{\\\"id\\\":\\\"5e157e606e6241f89f87c2c923e96c5b\\\",\\\"type\\\":1,\\\"protocol\\\":2,\\\"role\\\":0,\\\"remotePath\\\":[],\\\"extendInfo\\\":null}],\\\"extendInfo\\\":{\\\"dataAfterReduction\\\":39567470,\\\"dataBeforeReduction\\\":176705547,\\\"disk_info\\\":\\\"[\\\\n\\\\t{\\\\n\\\\t\\\\t\\\\\\\"customProperties\\\\\\\" : {},\\\\n\\\\t\\\\t\\\\\\\"datastoreName\\\\\\\" : \\\\\\\"autoDS_linux-CF\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"datastoreUrn\\\\\\\" : \\\\\\\"urn:sites:35B905D0:datastores:1\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"drExtParams\\\\\\\" : \\\\\\\"\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"indepDisk\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"ioMode\\\\\\\" : \\\\\\\"dataplane\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"ioWeight\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"isDiffVol\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"isThin\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"maxReadBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxReadRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxWriteBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxWriteRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"name\\\\\\\" : \\\\\\\"i-00000002-vda\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"pciType\\\\\\\" : \\\\\\\"VIRTIO\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"persistentDisk\\\\\\\" : true,\\\\n\\\\t\\\\t\\\\\\\"quantityGB\\\\\\\" : 40,\\\\n\\\\t\\\\t\\\\\\\"scsiCommandPassthrough\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"sequenceNum\\\\\\\" : 1,\\\\n\\\\t\\\\t\\\\\\\"siocFlag\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"srcVolumeUrn\\\\\\\" : \\\\\\\"null\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"status\\\\\\\" : \\\\\\\"USE\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"storageType\\\\\\\" : \\\\\\\"LOCALPOME\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"systemVolume\\\\\\\" : true,\\\\n\\\\t\\\\t\\\\\\\"totalRWBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"totalRWRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"type\\\\\\\" : \\\\\\\"normal\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"uri\\\\\\\" : \\\\\\\"/service/sites/35B905D0/volumes/57\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"urn\\\\\\\" : \\\\\\\"urn:sites:35B905D0:volumes:57\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"userUsedSize\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"uuid\\\\\\\" : \\\\\\\"b793aed2-cd61-44c2-b8a8-7ca8e341fb5d\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volNameOnDev\\\\\\\" : \\\\\\\"b793aed2-cd61-44c2-b8a8-7ca8e341fb5d\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volProvisionSize\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"volType\\\\\\\" : 1,\\\\n\\\\t\\\\t\\\\\\\"volumeFormat\\\\\\\" : \\\\\\\"qcow2\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volumeUrl\\\\\\\" : \\\\\\\"/POME/datastore_1/vol/vol_b793aed2-cd61-44c2-b8a8-7ca8e341fb5d/vol_7feadc34-7f56-476a-a95d-5c9af94e7742.img\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volumeUseType\\\\\\\" : 0\\\\n\\\\t},\\\\n\\\\t{\\\\n\\\\t\\\\t\\\\\\\"customProperties\\\\\\\" : {},\\\\n\\\\t\\\\t\\\\\\\"datastoreName\\\\\\\" : \\\\\\\"autoDS_linux-CF\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"datastoreUrn\\\\\\\" : \\\\\\\"urn:sites:35B905D0:datastores:1\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"drExtParams\\\\\\\" : \\\\\\\"\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"indepDisk\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"ioMode\\\\\\\" : \\\\\\\"dataplane\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"ioWeight\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"isDiffVol\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"isThin\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"maxReadBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxReadRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxWriteBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"maxWriteRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"name\\\\\\\" : \\\\\\\"c43f234b-306d-4d37-bdea-c06ec44cc6a2\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"pciType\\\\\\\" : \\\\\\\"VIRTIO\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"persistentDisk\\\\\\\" : true,\\\\n\\\\t\\\\t\\\\\\\"quantityGB\\\\\\\" : 1,\\\\n\\\\t\\\\t\\\\\\\"scsiCommandPassthrough\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"sequenceNum\\\\\\\" : 2,\\\\n\\\\t\\\\t\\\\\\\"siocFlag\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"srcVolumeUrn\\\\\\\" : \\\\\\\"null\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"status\\\\\\\" : \\\\\\\"USE\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"storageType\\\\\\\" : \\\\\\\"LOCALPOME\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"systemVolume\\\\\\\" : false,\\\\n\\\\t\\\\t\\\\\\\"totalRWBytes\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"totalRWRequest\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"type\\\\\\\" : \\\\\\\"normal\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"uri\\\\\\\" : \\\\\\\"/service/sites/35B905D0/volumes/175\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"urn\\\\\\\" : \\\\\\\"urn:sites:35B905D0:volumes:175\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"userUsedSize\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"uuid\\\\\\\" : \\\\\\\"c43f234b-306d-4d37-bdea-c06ec44cc6a2\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volNameOnDev\\\\\\\" : \\\\\\\"c43f234b-306d-4d37-bdea-c06ec44cc6a2\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volProvisionSize\\\\\\\" : 0,\\\\n\\\\t\\\\t\\\\\\\"volType\\\\\\\" : 1,\\\\n\\\\t\\\\t\\\\\\\"volumeFormat\\\\\\\" : \\\\\\\"qcow2\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volumeUrl\\\\\\\" : \\\\\\\"/POME/datastore_1/vol/vol_c43f234b-306d-4d37-bdea-c06ec44cc6a2/vol_3865cd7b-b03a-4e38-b6f1-7b9d7b87c0a0.img\\\\\\\",\\\\n\\\\t\\\\t\\\\\\\"volumeUseType\\\\\\\" : 0\\\\n\\\\t}\\\\n]\\\",\\\"fsRelations\\\":{\\\"relations\\\":[{\\\"newEsn\\\":\\\"2102354PBB10MC100001\\\",\\\"newFsId\\\":\\\"58\\\",\\\"newFsName\\\":\\\"Rep_247129_Huaage_VM_245af1ef-f79e-525e-8363-e54e0eab3175\\\",\\\"oldEsn\\\":\\\"2102354DFB10MA000001\\\",\\\"oldFsId\\\":\\\"136\\\",\\\"oldFsName\\\":\\\"VM_245af1ef-f79e-525e-8363-e54e0eab3175\\\",\\\"role\\\":0}]},\\\"multiFileSystem\\\":\\\"false\\\"},\\\"format\\\":0,\\\"backup_id\\\":\\\"8569e52b-231c-4ff5-9349-18127acfcc63\\\",\\\"archive_id\\\":{\\\"present\\\":true},\\\"storage_id\\\":\\\"5e157e606e6241f89f87c2c923e96c5b\\\"}\",\"resource_id\":\"245af1ef-f79e-525e-8363-e54e0eab3175\",\"resource_name\":\"Euler2.8_caijie001\",\"resource_type\":\"VM\",\"resource_sub_type\":\"FusionCompute\",\"resource_location\":\"8.40.98.196/ManagementCluster/Euler2.8_caijie001\",\"resource_status\":\"EXIST\",\"resource_properties\":\"{\\\"name\\\":\\\"Euler2.8_caijie001\\\",\\\"path\\\":\\\"8.40.98.196/ManagementCluster/Euler2.8_caijie001\\\",\\\"root_uuid\\\":\\\"ba3ede74-ab17-35bf-8066-fb1dc82e04b2\\\",\\\"parent_name\\\":\\\"ManagementCluster\\\",\\\"parent_uuid\\\":\\\"d9f56277-7442-5b17-b861-40385109c9e6\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"VM\\\",\\\"sub_type\\\":\\\"FusionCompute\\\",\\\"uuid\\\":\\\"245af1ef-f79e-525e-8363-e54e0eab3175\\\",\\\"created_time\\\":\\\"2022-09-07T14:52:54.972000\\\",\\\"ext_parameters\\\":{\\\"pre_script\\\":null,\\\"post_script\\\":null,\\\"failed_script\\\":null,\\\"agents\\\":\\\"f6129802-10bd-486f-8225-5c0f97d535a5\\\",\\\"overwrite\\\":null,\\\"binding_policy\\\":null,\\\"resource_filters\\\":null,\\\"disk_filters\\\":null,\\\"disk_info\\\":[]},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"38f81caf-6f1e-4c2f-b24c-ec9e2b4fcf5a\\\",\\\"sla_name\\\":\\\"fc-hr-replication\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":true,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"ba3ede74-ab17-35bf-8066-fb1dc82e04b2\\\",\\\"environment_name\\\":\\\"fc-arm\\\",\\\"environment_endpoint\\\":\\\"8.40.98.196\\\",\\\"environment_os_type\\\":null,\\\"environment_type\\\":\\\"Platform\\\",\\\"environment_sub_type\\\":\\\"FusionCompute\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":null,\\\"extendInfo\\\":{\\\"vGroup\\\":\\\"vGeneral\\\",\\\"numberOfDisk\\\":\\\"999999999\\\",\\\"machineDes\\\":\\\"e3ecdf1b-654b-4110-874d-09b0ebb45588\\\",\\\"storageSize\\\":\\\"42991616\\\",\\\"objectGuid\\\":\\\"245af1ef-f79e-525e-8363-e54e0eab3175\\\",\\\"machineGuid\\\":\\\"e3ecdf1b-654b-4110-874d-09b0ebb45588\\\",\\\"hostId\\\":\\\"\\\",\\\"machineOs\\\":\\\"EulerOS 2.8 64bit\\\",\\\"moReference\\\":\\\"/service/sites/35B905D0/vms/i-00000002\\\",\\\"group\\\":\\\"\\\",\\\"status\\\":\\\"stopped\\\"}}\",\"resource_environment_name\":\"fc-arm\",\"resource_environment_ip\":\"8.40.98.196\",\"sla_name\":\"hr-archive\",\"sla_properties\":\"{\\\"uuid\\\":\\\"1e973c1f-c9d0-4833-b36b-70c8b9c1c3c7\\\",\\\"name\\\":\\\"hr-archive\\\",\\\"created_time\\\":\\\"2022-09-12T17:20:44.782+08:00\\\",\\\"type\\\":1,\\\"application\\\":\\\"Replica\\\",\\\"policy_list\\\":[{\\\"uuid\\\":\\\"a1872e38-3ce5-4fba-9efc-3331eaec4898\\\",\\\"name\\\":\\\"策略0\\\",\\\"type\\\":\\\"archiving\\\",\\\"action\\\":\\\"archiving\\\",\\\"retention\\\":{\\\"retention_type\\\":2,\\\"retention_duration\\\":1,\\\"duration_unit\\\":\\\"d\\\",\\\"daily_copies\\\":null,\\\"weekly_copies\\\":null,\\\"monthly_copies\\\":null,\\\"yearly_copies\\\":null},\\\"schedule\\\":{\\\"trigger\\\":2,\\\"interval\\\":null,\\\"interval_unit\\\":null,\\\"start_time\\\":null,\\\"window_start\\\":null,\\\"window_end\\\":null,\\\"days_of_week\\\":null,\\\"days_of_month\\\":null,\\\"days_of_year\\\":null,\\\"trigger_action\\\":null},\\\"ext_parameters\\\":{\\\"qos_id\\\":\\\"\\\",\\\"protocol\\\":2,\\\"storage_id\\\":\\\"5e157e606e6241f89f87c2c923e96c5b\\\",\\\"archiving_scope\\\":\\\"all_no_archiving\\\",\\\"network_access\\\":true,\\\"auto_retry\\\":true,\\\"auto_retry_times\\\":3,\\\"archive_target_type\\\":1,\\\"auto_retry_wait_minutes\\\":5,\\\"alarm_after_failure\\\":false},\\\"active\\\":true,\\\"is_active\\\":true}],\\\"resource_count\\\":null,\\\"archival_count\\\":null,\\\"replication_count\\\":null,\\\"is_global\\\":false}\",\"user_id\":\"ecd04267a3a04b67a50e152e2023a8cf\",\"is_archived\":true,\"is_replicated\":true,\"detail\":null,\"name\":\"Euler2.8_caijie001_1663034537\",\"storage_id\":\"\",\"gn\":34,\"prev_copy_id\":\"8569e52b-231c-4ff5-9349-18127acfcc63\",\"next_copy_id\":null,\"prev_copy_gn\":33,\"next_copy_gn\":null}";
        return JSONObject.fromObject(repCopyJson).toBean(Copy.class);
    }
}

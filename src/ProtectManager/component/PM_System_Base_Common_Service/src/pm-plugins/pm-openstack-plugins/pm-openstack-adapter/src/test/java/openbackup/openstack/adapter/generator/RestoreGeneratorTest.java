package openbackup.openstack.adapter.generator;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.testdata.TestDataGenerator;

import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Test;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link RestoreGenerator} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
public class RestoreGeneratorTest {
    /**
     * 用例名称：验证生成CreateRestoreTaskRequest对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateCreateRestoreReq_givenServerRestoreJobWithActiveStatus()
            throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        ProtectedResource resource = createActiveProtectedResource();
        CreateRestoreTaskRequest req = RestoreGenerator.generateCreateRestoreReq(restoreJob, resource);
        assertThat(req.getCopyId()).isEqualTo(restoreJob.getCopyId());
        assertThat(req.getAgents()).isEmpty();
        assertThat(req.getTargetEnv()).isEqualTo(resource.getRootUuid());
        assertThat(req.getRestoreType()).isEqualTo(RestoreTypeEnum.CR);
        assertThat(req.getTargetLocation()).isEqualTo(RestoreLocationEnum.ORIGINAL);
        assertThat(req.getTargetObject()).isEqualTo(resource.getUuid());

        List<String> subObjects = req.getSubObjects();
        assertThat(subObjects).hasSize(2)
            .contains(
                "{\"extendInfo\":{\"targetVolume\":\"{\\\"size\\\":\\\"100\\\",\\\"id\\\":\\\"a7ac8fac-b904-4434-ad65-891744a3c5c3\\\"}\"},\"uuid\":\"a7ac8fac-b904-4434-ad65-891744a3c5c3\"}",
                "{\"extendInfo\":{\"targetVolume\":\"{\\\"size\\\":\\\"50\\\",\\\"id\\\":\\\"a7ac8fac-b904-4434-ad65-891744a3c000\\\"}\"},\"uuid\":\"a7ac8fac-b904-4434-ad65-891744a3c000\"}");

        Map<String, String> extendInfo = req.getExtendInfo();
        assertThat(extendInfo)
                .isNotEmpty()
                .containsEntry("copyVerify", Boolean.FALSE.toString())
                .containsEntry("restoreLocation", resource.getPath())
                .containsEntry("powerState", "1")
                .containsEntry("restore_name", restoreJob.getName())
                .containsEntry("description", restoreJob.getDescription())
                .containsEntry("restore_type", restoreJob.getType().getType())
                .containsEntry("instance_id", restoreJob.getInstanceId());
    }

    /**
     * 用例名称：验证生成CreateRestoreTaskRequest对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateCreateRestoreReq_givenVolumeRestoreJobWithStoppedStatus()
        throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createVolumeRestoreJob();
        ProtectedResource resource = createStoppedProtectedResource();
        CreateRestoreTaskRequest req = RestoreGenerator.generateCreateRestoreReq(restoreJob, resource);

        List<String> subObjects = req.getSubObjects();
        assertThat(subObjects).hasSize(1)
            .contains(
                "{\"extendInfo\":{\"targetVolume\":\"{\\\"size\\\":\\\"50\\\",\\\"id\\\":\\\"a7ac8fac-b904-4434-ad65-891744a3c000\\\"}\"},\"uuid\":\"a7ac8fac-b904-4434-ad65-891744a3c000\"}");
        Map<String, String> extendInfo = req.getExtendInfo();
        assertThat(extendInfo)
            .isNotEmpty()
            .containsEntry("copyVerify", Boolean.FALSE.toString())
            .containsEntry("restoreLocation", resource.getPath())
            .containsEntry("powerState", "0")
            .containsEntry("restore_name", restoreJob.getName())
            .containsEntry("description", restoreJob.getDescription())
            .containsEntry("restore_type", restoreJob.getType().getType())
            .containsEntry("instance_id", restoreJob.getInstanceId());
    }

    public static ProtectedResource createActiveProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid(UUIDGenerator.getUUID());
        resource.setUuid("07eab57c-f7c2-42fb-998f-c6d884964c53");
        resource.setPath("test path");

        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("volInfo",
            "[{\"id\":\"a7ac8fac-b904-4434-ad65-891744a3c5c3\",\"name\":\"VM001_SYSTEM_DISK\",\"bootable\":\"true\",\"size\":\"100\",\"shareable\":\"false\",\"architecture\":\"x86_64\"},{\"id\":\"a7ac8fac-b904-4434-ad65-891744a3c000\",\"name\":\"VM001_SYSTEM_DISK\",\"bootable\":\"true\",\"size\":\"50\",\"shareable\":\"false\",\"architecture\":\"x86_64\"}]");
        extendInfo.put("server", getResourceActiveServer());
        resource.setExtendInfo(extendInfo);
        return resource;
    }

    public static ProtectedResource createStoppedProtectedResource() {
        ProtectedResource resource = createActiveProtectedResource();
        resource.getExtendInfo().put("server", getResourceStoppedServer());
        return resource;
    }

    private static String getResourceActiveServer() {
        return "{\"id\":\"07eab57c-f7c2-42fb-998f-c6d884964c53\",\"name\":\"VM001\",\"projectId\":\"542ada72ff644cd2819446ada95920cb\",\"status\":\"active\"}";
    }

    private static String getResourceStoppedServer() {
        return "{\"id\":\"07eab57c-f7c2-42fb-998f-c6d884964c53\",\"name\":\"VM001\",\"projectId\":\"542ada72ff644cd2819446ada95920cb\",\"status\":\"stopped\"}";
    }
}

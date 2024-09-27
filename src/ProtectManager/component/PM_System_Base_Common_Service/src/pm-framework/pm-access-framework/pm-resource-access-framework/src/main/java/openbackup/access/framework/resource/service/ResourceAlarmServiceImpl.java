package openbackup.access.framework.resource.service;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceAlarmProvider;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.alarm.CommonAlarmService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * 功能描述
 *
 * @author y30044273
 * @since 2023-06-06
 */
@Service
@Slf4j
public class ResourceAlarmServiceImpl implements ResourceAlarmService {
    /**
     * 删除受保护资源的告警码
     */
    private static final String DELETE_PROTECTED_RESOURCE = "0x20640332003C";

    @Autowired
    ProtectedResourceRepository repository;

    @Autowired
    CommonAlarmService commonAlarmService;

    @Autowired
    private ProviderManager providerManager;

    @Override
    public void alarmDeleteProtectedResource(Set<String> uuids) {
        List<ProtectedObjectPo> protectedObjectPos = repository.queryProtectedObject(new ArrayList<>(uuids));
        if (VerifyUtil.isEmpty(protectedObjectPos)) {
            return;
        }

        for (ProtectedObjectPo protectedObjectPo : protectedObjectPos) {
            log.info("alarm: protected resource to be delete:{}, name:{}",
                    protectedObjectPo.getUuid(), protectedObjectPo.getName());
            ProtectedResource protectedResource = new ProtectedResource();
            protectedResource.setSubType(protectedObjectPo.getSubType());
            protectedResource.setUuid(protectedObjectPo.getResourceId());
            ResourceAlarmProvider provider =
                providerManager.findProvider(ResourceAlarmProvider.class, protectedResource, null);
            String resourceName = protectedObjectPo.getName();
            if (provider != null) {
                resourceName = provider.getAlarmResourceName(protectedResource);
            }
            generateHealthCheckAlarm(resourceName, protectedObjectPo.getResourceId());
        }
    }

    private void generateHealthCheckAlarm(String name, String resourceId) {
        commonAlarmService.generateAlarm(genHealthAlarm(name, resourceId));
    }

    private LegoInternalAlarm genHealthAlarm(String name, String resourceId) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmId(DELETE_PROTECTED_RESOURCE);
        legoInternalAlarm.setAlarmTime(System.currentTimeMillis() / IsmNumberConstant.THOUSAND);
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.ONE);
        legoInternalAlarm.setMoIp("127.0.0.1");
        legoInternalAlarm.setMoName("Resource");
        legoInternalAlarm.setAlarmParam(new String[] {name});
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.RESOURCE.getValue());
        legoInternalAlarm.setResourceId(resourceId);
        return legoInternalAlarm;
    }
}

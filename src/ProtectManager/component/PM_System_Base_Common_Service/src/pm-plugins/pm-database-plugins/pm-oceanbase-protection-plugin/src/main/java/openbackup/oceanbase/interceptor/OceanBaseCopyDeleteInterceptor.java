package openbackup.oceanbase.interceptor;

import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.LOG;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * OceanBase副本删除provider
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-11-24
 */
@Slf4j
@Component
public class OceanBaseCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * Constructor
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public OceanBaseCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(subType);
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("OceanBaseCopyDeleteInterceptor,handleTask,start.");
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (MapUtils.isEmpty(advanceParams)) {
            advanceParams = new HashMap<>();
        }
        advanceParams.put(OBConstants.RESOURCE_EXISTS, String.valueOf(super.isResourceExists(task)));
        task.setAdvanceParams(advanceParams);
    }

    /**
     * 删除全量副本时，如果当前副本之前有日志副本，则只删除当前副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        Optional<Copy> latestLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
            thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation());
        if (latestLogBackupCopy.isPresent()) {
            log.warn("exist log copy before this copy, uuid is {}", latestLogBackupCopy.get().getUuid());
            return Collections.emptyList();
        }

        int format = CopyUtil.getFormat(thisCopy).orElse(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        if (Objects.equals(format, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat())) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        if (Objects.equals(format, CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat())) {
            return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, Collections.singletonList(LOG));
        }
        return Collections.emptyList();
    }
}
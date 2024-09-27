package openbackup.tidb.resources.access.restore;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 表恢复
 *
 * @author w00426202
 * @since 2023-07-21
 */
@Slf4j
@Component
public class TidbTableRestoreInterceptor extends TidbDatabaseRestoreInterceptor {
    /**
     * 构造器
     *
     * @param tidbService tidbService
     * @param copyRestApi copyRestApi
     * @param tidbAgentProvider tidbAgentProvider
     * @param resourceService resourceService
     * @param defaultSelector defaultSelector
     */
    public TidbTableRestoreInterceptor(TidbService tidbService, TidbAgentProvider tidbAgentProvider,
        CopyRestApi copyRestApi, ResourceService resourceService, DefaultProtectAgentSelector defaultSelector) {
        super(tidbService, tidbAgentProvider, copyRestApi, resourceService, defaultSelector);
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_TABLE.getType().equals(object);
    }
}

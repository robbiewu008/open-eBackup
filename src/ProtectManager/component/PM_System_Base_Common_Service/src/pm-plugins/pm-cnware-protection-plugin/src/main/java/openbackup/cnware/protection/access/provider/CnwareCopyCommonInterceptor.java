package openbackup.cnware.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.cnware.protection.access.dto.CnwareVolInfo;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * CnwareCopyCommonInterceptor
 *
 * @author y30037959
 * @since 2024-08-20
 */
@Slf4j
@Component
public class CnwareCopyCommonInterceptor implements CopyCommonInterceptor {
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(resourceSubType);
    }

    @Override
    public void buildCatalogsRequest(Copy copy, CopyCatalogsRequest catalogsRequest) {
        String copyMetaData = CnwareVolInfo.convert2IndexDiskInfos(JSONObject.fromObject(copy.getProperties()));
        catalogsRequest.getCopyInfo().setCopyMetaData(copyMetaData);
    }
}

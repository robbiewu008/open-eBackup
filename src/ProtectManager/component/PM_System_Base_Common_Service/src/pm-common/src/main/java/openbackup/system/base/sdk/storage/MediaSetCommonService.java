package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;

/**
 * 介质集管理业务层
 *
 * @author z00633516
 * @since 2022-01-18
 */
public interface MediaSetCommonService {
    /**
     * 获取介质集详情
     *
     * @param mediaSetId 介质集UUID
     * @return 介质集详情
     */
    TapeSetDetailResponse getTapeSetDetail(String mediaSetId);
}

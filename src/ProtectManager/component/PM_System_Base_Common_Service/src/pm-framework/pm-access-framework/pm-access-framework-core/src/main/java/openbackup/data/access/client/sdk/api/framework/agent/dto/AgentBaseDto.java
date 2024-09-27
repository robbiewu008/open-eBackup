package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.Data;

/**
 * Agent Base Dto
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/2/28
 */
@Data
public class AgentBaseDto {
    private String errorCode;

    private String errorMessage;

    /**
     * 从Agent获取的值是否成功
     *
     * @return 接口是否返回成功
     */
    public boolean isAgentBaseDtoReturnSuccess() {
        return !VerifyUtil.isEmpty(errorCode) && Integer.parseInt(errorCode) == LegoNumberConstant.ZERO;
    }
}

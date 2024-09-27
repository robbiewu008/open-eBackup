package openbackup.data.protection.access.provider.sdk.base;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * SanClient的实体模型，方便以后扩展其他参数
 *
 * @author n30046257
 * @since 2023/7/11
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class SanClientInfo {
    private String id;

    private String ip;

    private int port;

    private String[] sanClientWwpns;

    private List<String> iqns;

    private String[] wwpns;

    private String[] fcPorts;

    private boolean isOpenLanFreeSwitch;
}

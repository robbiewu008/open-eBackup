package openbackup.system.base.sdk.repository.model;

import lombok.Data;

import java.util.List;

/**
 * 逻辑端口响应
 *
 * @author mwx776342
 * @since 2022-04-21
 */
@Data
public class LogicPortResponse {
    private List<String> logicPortList;
}

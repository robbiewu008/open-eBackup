package openbackup.data.access.framework.core.security.journal;

import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-10-10
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class LoggingParam {
    private String target;

    private String[] details;

    private boolean isSuccess;

    private LegoCheckedException legoCheckedException;
}

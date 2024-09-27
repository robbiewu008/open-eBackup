package openbackup.data.access.framework.livemount.controller.livemount.model;

import lombok.Data;

import javax.validation.Valid;

/**
 * LiveMount 迁移请求参数
 *
 * @author h30003246
 * @since 2020-12-31
 */
@Data
public class LiveMountMigrateRequest {
    @Valid
    private VMWareMigrateParam vmWareMigrateParam;
}

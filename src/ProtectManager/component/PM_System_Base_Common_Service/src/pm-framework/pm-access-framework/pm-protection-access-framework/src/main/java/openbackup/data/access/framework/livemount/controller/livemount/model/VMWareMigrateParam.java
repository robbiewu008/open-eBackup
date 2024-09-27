package openbackup.data.access.framework.livemount.controller.livemount.model;

import openbackup.data.access.framework.livemount.common.enums.VMWareMigrateDiskDatastoreType;

import lombok.Data;

import java.util.List;

import javax.validation.Valid;

/**
 * VMWare 迁移请求参数
 *
 * @author h30003246
 * @since 2020-12-31
 */
@Data
public class VMWareMigrateParam {
    private VMWareMigrateDiskDatastoreType diskDatastoreType;

    @Valid
    private Datastore vmxDatastore;

    @Valid
    private List<Disk> disk;
}

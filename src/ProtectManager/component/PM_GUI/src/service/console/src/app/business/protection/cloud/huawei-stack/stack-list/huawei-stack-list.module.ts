import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { HuaWeiStackListComponent } from './huawei-stack-list.component';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { AddTelnetComputeModule } from './add-telnet/add-telnet.module';
import { SelectDatabaseListModule } from './select-database-list/select-database-list.module';
import { CloudStackAdvancedParameterModule } from './stack-advanced-parameter/cloud-stack-advanced-parameter.module';
import { HCSHostSummaryModule } from './host-summary/host-summary.module';
import { ProjectSummaryModule } from './project-summary/project-summary.module';
import { TenantDetailModule } from './tenant-detail/tenant-detail.module';
import { HCSCopyDataModule } from './copy-data/hcs-copy-data.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [HuaWeiStackListComponent],
  imports: [
    BaseModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    AddTelnetComputeModule,
    SelectDatabaseListModule,
    CloudStackAdvancedParameterModule,
    HCSHostSummaryModule,
    HCSCopyDataModule,
    ProjectSummaryModule,
    TenantDetailModule,
    CustomTableSearchModule
  ],
  exports: [HuaWeiStackListComponent]
})
export class HuaWeiStackListModule {}

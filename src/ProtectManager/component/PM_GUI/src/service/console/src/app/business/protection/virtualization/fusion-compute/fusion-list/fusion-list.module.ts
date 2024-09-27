import { NgModule } from '@angular/core';
import { SelectInstanceDatabaseModule } from 'app/business/protection/host-app/sql-server/select-instance-database/select-instance-database.module';
import { BaseModule } from 'app/shared';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { FusionComputeCopyDataModule } from './copy-data/fusion-compute-copy-data.module';
import { FusionAdvancedParameterModule } from './fusion-advanced-parameter/fusion-advanced-parameter.module';
import { FusionClusterSummaryModule } from './fusion-cluster-summary/fusion-cluster-summary.module';
import { FusionHostSummaryModule } from './fusion-host-summary/fusion-host-summary.module';
import { FusionListComponent } from './fusion-list.component';
import { FusionSummaryModule } from './fusion-summary/fusion-summary.module';
import { SelectDatabaseListModule } from './select-database-list/select-database-list.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [FusionListComponent],
  imports: [
    BaseModule,
    FusionSummaryModule,
    FusionHostSummaryModule,
    FusionClusterSummaryModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    SelectDatabaseListModule,
    SelectInstanceDatabaseModule,
    FusionAdvancedParameterModule,
    FusionComputeCopyDataModule,
    CustomTableSearchModule
  ],
  exports: [FusionListComponent]
})
export class FusionListModule {}

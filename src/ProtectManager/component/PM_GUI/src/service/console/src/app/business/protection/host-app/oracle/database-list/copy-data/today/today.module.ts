import { NgModule } from '@angular/core';
import { OracleRestoreModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/today/oracle-restore/oracle-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataListModule } from 'app/shared/components/copy-data-list/copy-data-list.module';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { SummaryModule } from './summary/summary.module';
import { TodayComponent } from './today.component';

@NgModule({
  declarations: [TodayComponent],
  imports: [
    BaseModule,
    OracleRestoreModule,
    SummaryModule,
    CopyDataListModule,
    ManualMountModule
  ],
  exports: [TodayComponent]
})
export class TodayModule {}

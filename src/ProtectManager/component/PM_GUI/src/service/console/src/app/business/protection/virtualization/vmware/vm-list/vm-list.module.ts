import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AdvancedModule as HostAdvancedModule } from '../host/advanced/advanced.module';
import { SummaryModule } from '../summary/summary.module';
import { AdvancedModule as VmAdvancedModule } from '../vm/advanced/advanced.module';
import { CopyDataModule } from '../vm/copy-data/copy-data.module';
import { SelectObjectsModule } from '../vm/select-objects/select-objects.module';
import { SummaryModule as VmSummaryModule } from '../vm/summary/summary.module';
import { VmListComponent } from './vm-list.component';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [VmListComponent],
  imports: [
    BaseModule,
    VmSummaryModule,
    VmAdvancedModule,
    CopyDataModule,
    SelectObjectsModule,
    SummaryModule,
    HostAdvancedModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    CustomTableSearchModule
  ],
  exports: [VmListComponent]
})
export class VmListModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { CopyDataModule } from './copy-data/copy-data.module';
import { CreateFilesetModule } from './create-fileset/create-fileset.module';
import { FilesetRoutingModule } from './fileset-routing.module';
import { FilesetComponent } from './fileset.component';
import { SummaryModule } from './summary/summary.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { CreateFilesetTemplateModule } from './fileset-template-list/create-fileset-template/create-fileset-template.module';
import { FilesetRestoreModule } from './fileset-restore/fileset-restore.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
@NgModule({
  declarations: [FilesetComponent],
  imports: [
    CommonModule,
    SummaryModule,
    BaseModule,
    CopyDataModule,
    DetailModalModule,
    SlaServiceModule,
    FilesetRoutingModule,
    CreateFilesetModule,
    ProtectModule,
    FilesetRestoreModule,
    CreateFilesetTemplateModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    CustomTableSearchModule
  ],
  providers: [ResourceDetailService],
  exports: [FilesetComponent]
})
export class FilesetModule {}

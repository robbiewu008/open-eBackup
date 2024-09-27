import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { FilesetsComponent } from './filesets.component';
import { CopyDataModule } from './copy-data/copy-data.module';
import { CreateFilesetModule } from 'app/business/protection/host-app/fileset/create-fileset/create-fileset.module';
import { SummaryModule } from 'app/business/protection/host-app/fileset/summary/summary.module';

@NgModule({
  declarations: [FilesetsComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    CopyDataModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    CreateFilesetModule,
    SummaryModule
  ],
  exports: [FilesetsComponent]
})
export class FilesetsModule {}

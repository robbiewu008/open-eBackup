import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { GroupJobDetailModule } from 'app/business/protection/virtualization/virtualization-group/group-job-detail/group-job-detail.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { JobDetailModule } from './job-detail/job-detail.module';
import { JobTableComponent } from './job-table.component';
import { ModifyRemarksModule } from './modify-remarks/modify-remarks.module';
import { ModifyHandleModule } from './modify-handle/modify-handle.module';
import { BatchRetryComponent } from './batch-retry/batch-retry.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [JobTableComponent, BatchRetryComponent],
  imports: [
    CommonModule,
    BaseModule,
    JobDetailModule,
    ModifyRemarksModule,
    ModifyHandleModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    GroupJobDetailModule,
    AlertModule
  ],
  exports: [JobTableComponent]
})
export class JobTableModule {}

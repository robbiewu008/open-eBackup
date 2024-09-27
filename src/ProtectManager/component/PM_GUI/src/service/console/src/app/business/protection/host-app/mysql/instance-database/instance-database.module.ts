import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { InstanceDatabaseComponent } from './instance-database.component';
import { RegisterModule } from './register/register.module';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';

@NgModule({
  declarations: [InstanceDatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    RegisterModule,
    SummaryModule,
    CopyDataModule,
    ModifyRetentionPolicyModule
  ],
  exports: [InstanceDatabaseComponent]
})
export class InstanceDatabaseModule {}

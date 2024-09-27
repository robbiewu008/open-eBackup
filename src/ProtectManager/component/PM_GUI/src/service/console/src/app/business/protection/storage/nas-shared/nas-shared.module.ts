import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { NasSharedRoutingModule } from './nas-shared-routing.module';
import { NasSharedComponent } from './nas-shared.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { RegisterNasShareModule } from './register-nas-share/register-nas-share.module';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { RegisterGaussdbTModule } from '../../host-app/gaussdb-t/register-gaussdb-t/register-gaussdb-t.module';

@NgModule({
  declarations: [NasSharedComponent],
  imports: [
    CommonModule,
    NasSharedRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    ModifyRetentionPolicyModule,
    RegisterNasShareModule,
    SummaryModule,
    CopyDataModule,
    MultiClusterSwitchModule,
    RegisterGaussdbTModule
  ],
  exports: [NasSharedComponent]
})
export class NasSharedModule {}

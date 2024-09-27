import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared/base.module';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { CopyDataModule } from './copy-data/copy-data.module';
import { GaussdbTRoutingModule } from './gaussdb-t-routing.module';
import { GaussdbTComponent } from './gaussdb-t.component';
import { RegisterGaussdbTModule } from './register-gaussdb-t/register-gaussdb-t.module';
import { NasSharedModule } from '../../storage/nas-shared/nas-shared.module';
import { SummaryModule } from './summary/summary.module';

@NgModule({
  declarations: [GaussdbTComponent],
  imports: [
    CommonModule,
    GaussdbTRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    ModifyRetentionPolicyModule,
    RegisterGaussdbTModule,
    SummaryModule,
    CopyDataModule,
    MultiClusterSwitchModule,
    NasSharedModule
  ]
})
export class GaussdbTModule {}

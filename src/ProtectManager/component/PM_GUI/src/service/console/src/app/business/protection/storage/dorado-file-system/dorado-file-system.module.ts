import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DoradoFileSystemRoutingModule } from './dorado-file-system-routing.module';
import { DoradoFileSystemComponent } from './dorado-file-system.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [DoradoFileSystemComponent],
  imports: [
    CommonModule,
    DoradoFileSystemRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    ModifyRetentionPolicyModule,
    MultiClusterSwitchModule
  ],
  exports: [DoradoFileSystemComponent]
})
export class DoradoFileSystemModule {}

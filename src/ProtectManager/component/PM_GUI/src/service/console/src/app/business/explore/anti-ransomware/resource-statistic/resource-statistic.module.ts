import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyDataDetailModule } from 'app/shared/components/copy-data-detail/copy-data-detail.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AntiPolicyDetailModule } from '../../policy/anti-policy-setting/anti-policy/anti-policy-detail/anti-policy-detail.module';
import { DetectionRepicasListModule } from './detection-repicas-list/detection-repicas-list.module';
import { ResourceStatisticComponent } from './resource-statistic.component';

@NgModule({
  declarations: [ResourceStatisticComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CopyDataDetailModule,
    AntiPolicyDetailModule,
    DetectionRepicasListModule,
    MultiClusterSwitchModule
  ],
  exports: [ResourceStatisticComponent]
})
export class ResourceStatisticModule {}

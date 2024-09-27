import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DetectionModelListModule } from '../detection-model-list/detection-model-list.module';
import { ResourceStatisticModule } from '../resource-statistic/resource-statistic.module';
import { OverviewRoutingModule } from './overview-routing.module';
import { OverviewComponent } from './overview.component';

@NgModule({
  declarations: [OverviewComponent],
  imports: [
    CommonModule,
    BaseModule,
    OverviewRoutingModule,
    DetectionModelListModule,
    MultiClusterSwitchModule,
    ProTableModule,
    ResourceStatisticModule
  ]
})
export class OverviewModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ReportsListModule } from './reports-list/reports-list.module';
import { ReportsRoutingModule } from './reports-routing.module';
import { ReportsSubscriptionModule } from './reports-subscription/reports-subscription.module';
import { ReportsComponent } from './reports.component';

@NgModule({
  declarations: [ReportsComponent],
  imports: [
    CommonModule,
    ReportsRoutingModule,
    BaseModule,
    ReportsListModule,
    MultiClusterSwitchModule,
    ReportsSubscriptionModule
  ]
})
export class ReportsModule {}

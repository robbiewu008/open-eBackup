import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { InstanceDatabaseModule } from '../gaussdb-dws/instance-database/instance-database.module';
import { GaussdbForOpengaussRoutingModule } from './gaussdb-for-opengauss-routing.module';
import { GaussdbForOpengaussComponent } from './gaussdb-for-opengauss.component';
import { SummaryModule } from './summary/summary.module';
import { SummaryModule as ProjectSummaryModule } from './project-summary/summary.module';
@NgModule({
  declarations: [GaussdbForOpengaussComponent],
  imports: [
    CommonModule,
    GaussdbForOpengaussRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    SummaryModule,
    ProjectSummaryModule,
    MultiClusterSwitchModule
  ]
})
export class GaussdbForOpengaussModule {}

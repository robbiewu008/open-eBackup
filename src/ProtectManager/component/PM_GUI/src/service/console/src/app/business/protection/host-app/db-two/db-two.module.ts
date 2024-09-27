import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { InstanceDatabaseModule } from '../gaussdb-dws/instance-database/instance-database.module';
import { DbTwoRoutingModule } from './db-two-routing.module';
import { DbTwoComponent } from './db-two.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SummaryModule } from './summary/summary.module';
import { SummaryModule as DbTwoSummaryModule } from './db-two-summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { SummaryModule as DbTwoDatabaseListComponent } from './database-summary/summary.module';
@NgModule({
  declarations: [DbTwoComponent],
  imports: [
    CommonModule,
    DbTwoRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    MultiClusterSwitchModule,
    SummaryModule,
    DbTwoSummaryModule,
    DbTwoDatabaseListComponent,
    CopyDataModule
  ]
})
export class DbTwoModule {}

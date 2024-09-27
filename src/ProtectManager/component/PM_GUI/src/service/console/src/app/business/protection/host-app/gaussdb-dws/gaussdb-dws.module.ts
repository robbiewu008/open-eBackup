import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { InstanceDatabaseModule } from './instance-database/instance-database.module';
import { GaussdbDWSRoutingModule } from './gaussdb-dws-routing.module';
import { GaussdbDWSComponent } from './gaussdb-dws.component';
import { CopyDataModule } from './copy-data/copy-data.module';
import { SummaryModule } from './instance-database/summary/summary.module';
import { ClusterRestoreModule } from './restore-cluster/restore-cluster.module';
@NgModule({
  declarations: [GaussdbDWSComponent],
  imports: [
    CommonModule,
    GaussdbDWSRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    CopyDataModule,
    SummaryModule,
    MultiClusterSwitchModule,
    ClusterRestoreModule
  ]
})
export class GaussdbDWSModule {}

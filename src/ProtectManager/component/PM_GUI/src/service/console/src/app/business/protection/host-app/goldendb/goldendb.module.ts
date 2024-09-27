import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { InstanceDatabaseModule } from '../gaussdb-dws/instance-database/instance-database.module';
import { GoldendbRoutingModule } from './goldendb-routing.module';
import { GoldendbComponent } from './goldendb.component';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterBackupsetDetailModule } from './cluster-backupset-detail/cluster-backupset-detail.module';
import { SummaryModule } from './summary/summary.module';
import { GoldendbRestoreModule } from './goldendb-restore/goldendb-restore.module';
@NgModule({
  declarations: [GoldendbComponent],
  imports: [
    CommonModule,
    GoldendbRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    ClusterDetailModule,
    ClusterBackupsetDetailModule,
    SummaryModule,
    MultiClusterSwitchModule,
    GoldendbRestoreModule
  ]
})
export class GoldendbModule {}

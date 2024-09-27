import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ClustersModule } from './clusters/clusters.module';
import { HbaseRoutingModule } from './hbase-routing.module';
import { HbaseComponent } from './hbase.component';
import { BackupSetModule } from './backup-set/backup-set.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [HbaseComponent],
  imports: [
    CommonModule,
    HbaseRoutingModule,
    BaseModule,
    ClustersModule,
    BackupSetModule,
    MultiClusterSwitchModule
  ]
})
export class HbaseModule {}

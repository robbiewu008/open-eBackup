import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { HiveRoutingModule } from './hive-routing.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { HiveComponent } from './hive.component';
import { ClustersModule } from '../hbase/clusters/clusters.module';
import { BackupSetModule } from '../hbase/backup-set/backup-set.module';

@NgModule({
  declarations: [HiveComponent],
  imports: [
    CommonModule,
    HiveRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ClustersModule,
    BackupSetModule
  ]
})
export class HiveModule {}

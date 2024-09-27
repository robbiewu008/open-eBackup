import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HiveRoutingModule } from './elastic-search-routing.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ElasticSearchComponent } from './elastic-search.component';
import { ClustersModule } from '../hbase/clusters/clusters.module';
import { BackupSetModule } from '../hbase/backup-set/backup-set.module';
import { CreateBackupsetModule } from './create-backupset/create-backupset.module';
@NgModule({
  declarations: [ElasticSearchComponent],
  imports: [
    CommonModule,
    HiveRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ClustersModule,
    BackupSetModule,
    CreateBackupsetModule
  ]
})
export class ElasticSearchModule {}

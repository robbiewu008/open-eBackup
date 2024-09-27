import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ClustersComponent } from './clusters.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterBackupsetDetailModule } from './cluster-backupset-detail/cluster-backupset-detail.module';
import { RegisterClusterModule } from '../../hdfs/clusters/register-cluster/register-cluster.module';
import { RegisterClusterModule as HiveRegisterClusterModule } from '../../hive/register-cluster/register-cluster.module';
import { RegisterClusterModule as EsRegisterClusterComponent } from '../../elasticSearch/register-cluster/register-cluster.module';
@NgModule({
  declarations: [ClustersComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ClusterDetailModule,
    ClusterBackupsetDetailModule,
    RegisterClusterModule,
    HiveRegisterClusterModule,
    EsRegisterClusterComponent
  ],
  exports: [ClustersComponent]
})
export class ClustersModule {}

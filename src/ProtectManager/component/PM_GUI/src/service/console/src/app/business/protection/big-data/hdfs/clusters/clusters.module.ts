import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ClustersComponent } from './clusters.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';

@NgModule({
  declarations: [ClustersComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    RegisterClusterModule,
    ClusterDetailModule
  ],
  exports: [ClustersComponent]
})
export class ClustersModule {}

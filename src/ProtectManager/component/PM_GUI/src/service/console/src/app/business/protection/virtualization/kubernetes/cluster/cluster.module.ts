import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterComponent } from './cluster.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { ClustreDetailModule } from './clustre-detail/clustre-detail.module';
import { CreateClusterModule } from '../../kubernetes-container/create-cluster/create-cluster.module';

@NgModule({
  declarations: [ClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    RegisterClusterModule,
    ClustreDetailModule,
    CreateClusterModule
  ],
  exports: [ClusterComponent]
})
export class ClusterModule {}

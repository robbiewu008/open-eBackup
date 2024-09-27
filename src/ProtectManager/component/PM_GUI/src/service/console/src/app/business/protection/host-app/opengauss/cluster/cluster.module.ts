import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterComponent } from './cluster.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { DetailModule } from './detail/detail.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';

@NgModule({
  declarations: [ClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    DetailModule,
    RegisterClusterModule
  ],
  exports: [ClusterComponent]
})
export class ClusterModule {}

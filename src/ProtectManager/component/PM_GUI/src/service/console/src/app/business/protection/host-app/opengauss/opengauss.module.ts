import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { OpengaussComponent } from './opengauss.component';
import { OpengaussRoutingModule } from './opengauss-routing.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { InstanceModule } from './instance/instance.module';
import { DatabaseModule } from './database/database.module';
import { ClusterModule } from './cluster/cluster.module';

@NgModule({
  declarations: [OpengaussComponent],
  imports: [
    CommonModule,
    OpengaussRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    InstanceModule,
    DatabaseModule,
    ClusterModule
  ]
})
export class OpengaussModule {}

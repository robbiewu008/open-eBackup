import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ClusterModule } from './cluster/cluster.module';
import { InstanceDatabaseModule } from './instance-database/instance-database.module';
import { MysqlRoutingModule } from './mysql-routing.module';
import { MysqlComponent } from './mysql.component';

@NgModule({
  declarations: [MysqlComponent],
  imports: [
    CommonModule,
    MysqlRoutingModule,
    BaseModule,
    ClusterModule,
    InstanceDatabaseModule,
    MultiClusterSwitchModule
  ]
})
export class MysqlModule {}

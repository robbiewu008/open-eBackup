import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ClusterModule } from '../mysql/cluster/cluster.module';
import { InstanceDatabaseModule } from '../mysql/instance-database/instance-database.module';
import { PostgreClusterModule } from './cluster/postgre-cluster.module';
import { PostgreCopyDataModule } from './instance-database/copy-data/postgre-copy-data.module';
import { PostgreInstanceDatabaseModule } from './instance-database/postgre-instance-database.module';
import { PostgreSqlRoutingModule } from './postgre-sql-routing.module';
import { PostgreSqlComponent } from './postgre-sql.component';

@NgModule({
  declarations: [PostgreSqlComponent],
  imports: [
    CommonModule,
    PostgreSqlRoutingModule,
    BaseModule,
    ClusterModule,
    InstanceDatabaseModule,
    MultiClusterSwitchModule,
    PostgreClusterModule,
    PostgreInstanceDatabaseModule,
    PostgreCopyDataModule
  ]
})
export class PostgreSqlModule {}

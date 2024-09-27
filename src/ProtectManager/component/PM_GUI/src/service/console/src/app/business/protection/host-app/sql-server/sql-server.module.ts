import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { InstanceDatabaseModule } from '../gaussdb-dws/instance-database/instance-database.module';
import { SQLServerRoutingModule } from './sql-server-routing.module';
import { SQLServerComponent } from './sql-server.component';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SummaryModule } from './summary/summary.module';
import { SelectInstanceDatabaseModule } from './select-instance-database/select-instance-database.module';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterBackupsetDetailModule } from './cluster-backupset-detail/cluster-backupset-detail.module';
@NgModule({
  declarations: [SQLServerComponent],
  imports: [
    CommonModule,
    SQLServerRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    RegisterInstanceModule,
    MultiClusterSwitchModule,
    SelectInstanceDatabaseModule,
    ClusterDetailModule,
    ClusterBackupsetDetailModule,
    SummaryModule
  ]
})
export class SQLServerModule {}

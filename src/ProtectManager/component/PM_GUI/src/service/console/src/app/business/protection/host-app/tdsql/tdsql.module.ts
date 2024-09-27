import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { TdsqlRoutingModule } from './tdsql-routing.module';
import { TdsqlComponent } from './tdsql.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { SummaryModule } from './summary-instance/summary.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryInstanceListModule } from './summary-instance-list/summary-instance-list.module';
import { SummaryModule as DistributedSummaryModule } from './dirstibuted-instance/summary/summary.module';
import { RegisterDistributedInstanceModule } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/register-distributed-instance/register-distributed-instance.module';
import { RestoreModule as DistributedRestore } from './dirstibuted-instance/restore/restore.module';
@NgModule({
  declarations: [TdsqlComponent],
  imports: [
    CommonModule,
    TdsqlRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    RegisterClusterModule,
    RegisterInstanceModule,
    SummaryModule,
    SummaryClusterModule,
    SummaryInstanceListModule,
    CopyDataModule,
    RegisterDistributedInstanceModule,
    DistributedSummaryModule,
    DistributedRestore
  ]
})
export class TdsqlModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { TidbComponent } from './tidb.component';
import { TidbRoutingModule } from './tidb-routing.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterDatabaseModule } from './register-database/register-database.module';
import { RegisterTableModule } from './register-table/register-table.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryDatabaseModule } from './summary-database/summary-database.module';
import { SummaryTableModule } from './summary-table/summary-table.module';
import { TidbRestoreModule } from './tidb-restore/tidb-restore.module';
import { AdvancedParameterModule } from './advanced-parameter/advanced-parameter.module';

@NgModule({
  declarations: [TidbComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    CopyDataModule,
    TidbRoutingModule,
    RegisterClusterModule,
    RegisterDatabaseModule,
    RegisterTableModule,
    SummaryClusterModule,
    SummaryDatabaseModule,
    SummaryTableModule,
    TidbRestoreModule,
    AdvancedParameterModule
  ]
})
export class TidbModule {}

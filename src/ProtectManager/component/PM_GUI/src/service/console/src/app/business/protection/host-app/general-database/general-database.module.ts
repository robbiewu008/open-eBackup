import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { GeneralDatabaseRoutingModule } from './general-database-routing.module';
import { GeneralDatabaseComponent } from './general-database.component';
import { TableTemplateModule } from './table-template/table-template.module';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';
@NgModule({
  declarations: [GeneralDatabaseComponent],
  imports: [
    CommonModule,
    GeneralDatabaseRoutingModule,
    BaseModule,
    TableTemplateModule,
    SummaryModule,
    CopyDataModule,
    MultiClusterSwitchModule
  ]
})
export class GeneralDatabaseModule {}

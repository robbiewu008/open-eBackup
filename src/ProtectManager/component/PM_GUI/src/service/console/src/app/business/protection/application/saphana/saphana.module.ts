import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { CopyDataModule } from '../../host-app/database-template/copy-data/copy-data.module';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { BaseModule } from 'app/shared';
import { SaphanaRoutingModule } from './saphana-routing.module';
import { SaphanaComponent } from './saphana.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { RegisterDatabaseModule } from './register-database/register-database.module';
import { SummaryInstanceModule } from './summary-instance/summary-instance.module';
import { SummaryDatabaseListModule } from './summary-database-list/summary-database-list.module';
import { SummaryDatabaseModule } from './summary-database/summary-database.module';

@NgModule({
  declarations: [SaphanaComponent],
  imports: [
    CommonModule,
    SaphanaRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    RegisterInstanceModule,
    RegisterDatabaseModule,
    SummaryInstanceModule,
    SummaryDatabaseListModule,
    SummaryDatabaseModule,
    CopyDataModule
  ]
})
export class SaphanaModule {}

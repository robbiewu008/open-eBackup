import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { InformixRoutingModule } from './informix-routing.module';
import { InformixComponent } from './informix.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SummaryModule } from './summary-instance/summary.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { SummaryServiceModule } from './summary-service/summary-service.module';
@NgModule({
  declarations: [InformixComponent],
  imports: [
    CommonModule,
    InformixRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    RegisterClusterModule,
    RegisterInstanceModule,
    SummaryModule,
    SummaryServiceModule,
    CopyDataModule
  ]
})
export class InformixModule {}

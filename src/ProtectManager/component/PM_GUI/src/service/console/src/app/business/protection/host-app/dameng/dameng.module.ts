import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DamengComponent } from './dameng.component';
import { DamengRoutingModule } from './dameng-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryModule } from './summary/summary.module';
import { RegisterModule } from './register/register.module';
import { CopyDataModule } from './copy-data/copy-data.module';

@NgModule({
  declarations: [DamengComponent],
  imports: [
    CommonModule,
    DamengRoutingModule,
    MultiClusterSwitchModule,
    BaseModule,
    ProButtonModule,
    ProTableModule,
    SummaryModule,
    RegisterModule,
    CopyDataModule
  ]
})
export class DamengModule {}

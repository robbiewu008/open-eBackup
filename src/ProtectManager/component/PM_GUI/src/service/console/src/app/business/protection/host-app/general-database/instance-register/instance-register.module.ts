import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceRegisterComponent } from './instance-register.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { AddHostModule } from './add-host/add-host.module';
import { AgentsJumperTipsModule } from 'app/shared/components/agents-jumper-tips.component';

@NgModule({
  declarations: [InstanceRegisterComponent],
  imports: [
    CommonModule,
    BaseModule,
    AgentsJumperTipsModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    AddHostModule
  ]
})
export class InstanceRegisterModule {}

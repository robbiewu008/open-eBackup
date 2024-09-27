import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { RegisterTenantComponent } from './register-tenant.component';
import { AgentsJumperTipsModule } from 'app/shared/components/agents-jumper-tips.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [RegisterTenantComponent],
  imports: [
    CommonModule,
    BaseModule,
    AgentsJumperTipsModule,
    ProButtonModule,
    ProTableModule,
    ProStatusModule
  ]
})
export class RegisterTenantModule {}

import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterDatabaseComponent } from './register-database.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { AgentsJumperTipsModule } from 'app/shared/components/agents-jumper-tips.component';

@NgModule({
  declarations: [RegisterDatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    AgentsJumperTipsModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class RegisterDatabaseModule {}

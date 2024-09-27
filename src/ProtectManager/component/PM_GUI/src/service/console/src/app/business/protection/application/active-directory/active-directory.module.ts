import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ActiveDirectoryComponent } from './active-directory.component';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { ActiveDirectoryRoutingModule } from './active-directory-routing.module';
import { RegisterModule } from './register/register.module';
import { SummaryModule } from './summary/summary.module';
import { RestoreModule } from './restore/restore.module';

@NgModule({
  declarations: [ActiveDirectoryComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    ActiveDirectoryRoutingModule,
    RegisterModule,
    SummaryModule,
    RestoreModule
  ]
})
export class ActiveDirectoryModule {}

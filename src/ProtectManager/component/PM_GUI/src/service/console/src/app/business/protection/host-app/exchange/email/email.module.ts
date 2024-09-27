import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../database-template/database-template.module';
import { EmailComponent } from './email.component';
import { EmailLevelRestoreModule } from 'app/business/protection/host-app/exchange/email/email-level-restore/email-level-restore.module';

@NgModule({
  declarations: [EmailComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    EmailLevelRestoreModule
  ],
  exports: [EmailComponent]
})
export class EmailModule {}

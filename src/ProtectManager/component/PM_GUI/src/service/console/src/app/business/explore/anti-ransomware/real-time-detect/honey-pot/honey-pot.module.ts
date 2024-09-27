import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { FileDetailModule } from './file-detail/file-detail.module';
import { HoneyPotComponent } from './honey-pot.component';
import { HoneypotFileWarningModule } from './honeypot-file-warning/honeypot-file-warning.module';
import { HoneypotSettingModule } from './honeypot-setting/honeypot-setting.module';

@NgModule({
  declarations: [HoneyPotComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProStatusModule,
    ProTableModule,
    ProButtonModule,
    HoneypotSettingModule,
    FileDetailModule,
    AlertModule,
    HoneypotFileWarningModule
  ],
  exports: [HoneyPotComponent]
})
export class HoneyPotModule {}

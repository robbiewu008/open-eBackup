import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { HoneypotSettingComponent } from './honeypot-setting.component';

@NgModule({
  declarations: [HoneypotSettingComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProStatusModule,
    ProTableModule,
    AlertModule
  ],
  exports: [HoneypotSettingComponent]
})
export class HoneypotSettingModule {}

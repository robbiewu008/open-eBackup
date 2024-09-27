import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { HoneypotFileWarningComponent } from './honeypot-file-warning.component';

@NgModule({
  declarations: [HoneypotFileWarningComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [HoneypotFileWarningComponent]
})
export class HoneypotFileWarningModule {}

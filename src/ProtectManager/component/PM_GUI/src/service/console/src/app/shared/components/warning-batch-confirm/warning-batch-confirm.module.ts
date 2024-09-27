import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { WarningBatchConfirmComponent } from './warning-batch-confirm.component';

@NgModule({
  declarations: [WarningBatchConfirmComponent],
  imports: [CommonModule, BaseModule],
  exports: [WarningBatchConfirmComponent]
})
export class WarningBatchConfirmModule {}

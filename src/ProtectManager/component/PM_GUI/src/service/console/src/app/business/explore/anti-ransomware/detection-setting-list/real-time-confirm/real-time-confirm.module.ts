import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { RealTimeConfirmComponent } from './real-time-confirm.component';
import { CheckboxModule, TooltipModule } from '@iux/live';

@NgModule({
  imports: [CommonModule, CheckboxModule, TooltipModule, BaseModule],
  declarations: [RealTimeConfirmComponent],
  exports: [RealTimeConfirmComponent]
})
export class RealTimeConfirmModule {}

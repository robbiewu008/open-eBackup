import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProtectParameterDetailComponent } from './protect-parameter-detail.component';

@NgModule({
  declarations: [ProtectParameterDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [ProtectParameterDetailComponent]
})
export class ProtectParameterDetailModule {}

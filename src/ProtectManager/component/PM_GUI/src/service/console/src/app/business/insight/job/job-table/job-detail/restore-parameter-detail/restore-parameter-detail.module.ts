import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RestoreParameterDetailComponent } from './restore-parameter-detail.component';

@NgModule({
  declarations: [RestoreParameterDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [RestoreParameterDetailComponent]
})
export class RestoreParameterDetailModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ThresholdModifyComponent } from './threshold-modify.component';

@NgModule({
  declarations: [ThresholdModifyComponent],
  imports: [CommonModule, BaseModule],

  exports: [ThresholdModifyComponent]
})
export class ThresholdModifyModule {}

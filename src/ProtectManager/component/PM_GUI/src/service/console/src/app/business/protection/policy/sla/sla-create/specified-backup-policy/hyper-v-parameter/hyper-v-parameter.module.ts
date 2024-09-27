import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { HyperVParameterComponent } from './hyper-v-parameter.component';

@NgModule({
  declarations: [HyperVParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [HyperVParameterComponent]
})
export class HyperVParameterModule {}

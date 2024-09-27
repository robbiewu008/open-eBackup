import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AdvancedParameterComponent } from './advanced-parameter.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedParameterComponent]
})
export class AdvancedParameterModule {}

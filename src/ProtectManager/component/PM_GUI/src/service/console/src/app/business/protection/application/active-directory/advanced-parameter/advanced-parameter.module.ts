import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';
import { AdvancedParameterComponent } from './advanced-parameter.component';

@NgModule({
  declarations: [AdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedParameterComponent]
})
export class AdvancedParameterModule {}

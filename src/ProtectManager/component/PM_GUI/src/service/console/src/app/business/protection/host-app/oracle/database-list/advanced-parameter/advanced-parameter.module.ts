import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AdvancedParameterComponent } from './advanced-parameter.component';

@NgModule({
  declarations: [AdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedParameterComponent]
})
export class AdvancedParameterModule {}

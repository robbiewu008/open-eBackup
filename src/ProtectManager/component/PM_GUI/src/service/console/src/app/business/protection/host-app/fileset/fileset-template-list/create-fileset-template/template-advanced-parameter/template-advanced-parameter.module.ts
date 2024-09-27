import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TemplateAdvancedParameterComponent } from './template-advanced-parameter.component';

@NgModule({
  declarations: [TemplateAdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [TemplateAdvancedParameterComponent]
})
export class TemplateAdvancedParameterModule {}

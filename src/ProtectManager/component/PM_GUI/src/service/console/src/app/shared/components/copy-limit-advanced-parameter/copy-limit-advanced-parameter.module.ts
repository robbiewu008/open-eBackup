import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CopyLimitAdvancedParameterComponent } from './copy-limit-advanced-parameter.component';

@NgModule({
  declarations: [CopyLimitAdvancedParameterComponent],
  imports: [CommonModule, BaseModule],
  exports: [CopyLimitAdvancedParameterComponent]
})
export class CopyLimitAdvancedParameterModule {}

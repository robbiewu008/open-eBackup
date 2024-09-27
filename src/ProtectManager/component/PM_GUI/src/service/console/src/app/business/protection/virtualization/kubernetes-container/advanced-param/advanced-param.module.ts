import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AdvancedParamComponent } from './advanced-param.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AdvancedParamComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedParamComponent]
})
export class AdvancedParamModule {}

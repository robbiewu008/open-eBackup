import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AdvancedParameterComponent } from './advanced-parameter.component';
import { BaseModule } from 'app/shared';
import { ProtectFilterModule } from 'app/shared/components/protect-filter/protect-filter.module';

@NgModule({
  declarations: [AdvancedParameterComponent],
  imports: [CommonModule, BaseModule, ProtectFilterModule],
  exports: [AdvancedParameterComponent]
})
export class AdvancedParameterModule {}

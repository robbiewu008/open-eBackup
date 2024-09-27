import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProtectFilterModule } from 'app/shared/components/protect-filter/protect-filter.module';
import { ApsAdvanceParameterComponent } from './aps-advance-parameter.component';

@NgModule({
  declarations: [ApsAdvanceParameterComponent],
  imports: [CommonModule, BaseModule, ProtectFilterModule],
  exports: [ApsAdvanceParameterComponent]
})
export class ApsAdvanceParameterModule {}

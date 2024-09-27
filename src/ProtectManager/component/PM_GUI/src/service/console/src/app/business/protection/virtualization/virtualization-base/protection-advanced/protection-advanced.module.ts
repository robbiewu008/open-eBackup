import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyLimitAdvancedParameterModule } from 'app/shared/components/copy-limit-advanced-parameter/copy-limit-advanced-parameter.module';
import { ProtectFilterModule } from 'app/shared/components/protect-filter/protect-filter.module';
import { ProtectionAdvancedComponent } from './protection-advanced.component';
import { UpdateIndexModule } from 'app/shared/components/update-index/update-index.module';

@NgModule({
  declarations: [ProtectionAdvancedComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProtectFilterModule,
    CopyLimitAdvancedParameterModule,
    UpdateIndexModule
  ],
  exports: [ProtectionAdvancedComponent]
})
export class ProtectionAdvancedModule {}

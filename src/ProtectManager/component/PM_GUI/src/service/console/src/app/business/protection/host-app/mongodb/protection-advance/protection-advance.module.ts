import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProtectionAdvanceComponent } from './protection-advance.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ProtectionAdvanceComponent],
  imports: [CommonModule, BaseModule],
  exports: [ProtectionAdvanceComponent]
})
export class ProtectionAdvanceModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateUpdatePolicyComponent } from './create-update-policy.component';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';

@NgModule({
  declarations: [CreateUpdatePolicyComponent],
  imports: [CommonModule, BaseModule, CurrentSystemTimeModule],
  exports: [CreateUpdatePolicyComponent]
})
export class CreateUpdatePolicyModule {}

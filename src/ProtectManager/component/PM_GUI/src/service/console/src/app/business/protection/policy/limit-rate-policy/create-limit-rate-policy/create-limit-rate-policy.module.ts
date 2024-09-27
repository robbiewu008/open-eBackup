import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CreateLimitRatePolicyComponent } from './create-limit-rate-policy.component';

@NgModule({
  declarations: [CreateLimitRatePolicyComponent],
  imports: [CommonModule, BaseModule],
  providers: [CreateLimitRatePolicyComponent]
})
export class CreateLimitRatePolicyModule {}

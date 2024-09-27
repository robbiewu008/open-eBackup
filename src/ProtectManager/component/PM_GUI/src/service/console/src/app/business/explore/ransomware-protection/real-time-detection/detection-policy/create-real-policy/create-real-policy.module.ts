import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateRealPolicyComponent } from './create-real-policy.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CreateRealPolicyComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateRealPolicyModule {}

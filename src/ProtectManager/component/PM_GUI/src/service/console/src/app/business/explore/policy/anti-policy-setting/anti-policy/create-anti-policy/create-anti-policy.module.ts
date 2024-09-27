import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateAntiPolicyComponent } from './create-anti-policy.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [CreateAntiPolicyComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule]
})
export class CreateAntiPolicyModule {}

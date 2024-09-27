import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AntiPolicyDetailComponent } from './anti-policy-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [AntiPolicyDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule]
})
export class AntiPolicyDetailModule {}

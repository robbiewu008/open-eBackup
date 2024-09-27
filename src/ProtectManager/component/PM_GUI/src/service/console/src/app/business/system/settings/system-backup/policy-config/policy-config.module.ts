import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { PolicyConfigComponent } from './policy-config.component';

@NgModule({
  declarations: [PolicyConfigComponent],
  imports: [CommonModule, BaseModule]
})
export class PolicyConfigModule {}

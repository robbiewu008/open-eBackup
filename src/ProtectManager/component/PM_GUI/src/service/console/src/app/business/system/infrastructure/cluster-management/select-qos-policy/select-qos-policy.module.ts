import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectQosPolicyComponent } from './select-qos-policy.component';

@NgModule({
  declarations: [SelectQosPolicyComponent],
  imports: [CommonModule, BaseModule],
  exports: [SelectQosPolicyComponent]
})
export class SelectQosPolicyModule {}

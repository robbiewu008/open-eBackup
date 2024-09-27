import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { UpdatePolicyDetailComponent } from './update-policy-detail.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';

@NgModule({
  imports: [CommonModule, BaseModule, CustomModalOperateModule],
  declarations: [UpdatePolicyDetailComponent],
  exports: [UpdatePolicyDetailComponent]
})
export class UpdatePolicyDetailModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ModifyRetentionPolicyComponent } from './modify-retention-policy.component';

@NgModule({
  declarations: [ModifyRetentionPolicyComponent],
  imports: [CommonModule, BaseModule],

  exports: [ModifyRetentionPolicyComponent]
})
export class ModifyRetentionPolicyModule {}

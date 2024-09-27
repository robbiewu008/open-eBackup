import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CopyLimitAdvancedParameterModule } from '../copy-limit-advanced-parameter/copy-limit-advanced-parameter.module';
import { ReplicaAdvancedParameterComponent } from './replica-advanced-parameter.component';

@NgModule({
  declarations: [ReplicaAdvancedParameterComponent],
  imports: [CommonModule, BaseModule, CopyLimitAdvancedParameterModule],
  exports: [ReplicaAdvancedParameterComponent]
})
export class ReplicaAdvancedParameterModule {}

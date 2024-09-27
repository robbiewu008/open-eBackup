import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ModifyAlarmThresholdComponent } from './modify-alarm-threshold.component';

@NgModule({
  declarations: [ModifyAlarmThresholdComponent],
  imports: [CommonModule, BaseModule]
})
export class ModifyAlarmThresholdModule {}

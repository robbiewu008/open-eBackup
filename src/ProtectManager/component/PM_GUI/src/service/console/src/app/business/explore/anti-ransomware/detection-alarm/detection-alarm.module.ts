import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionAlarmComponent } from './detection-alarm.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DetectionAlarmComponent],
  imports: [CommonModule, BaseModule],
  exports: [DetectionAlarmComponent]
})
export class DetectionAlarmModule {}

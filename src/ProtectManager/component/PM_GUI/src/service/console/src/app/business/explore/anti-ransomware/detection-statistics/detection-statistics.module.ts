import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionStatisticsComponent } from './detection-statistics.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DetectionStatisticsComponent],
  imports: [CommonModule, BaseModule],
  exports: [DetectionStatisticsComponent]
})
export class DetectionStatisticsModule {}

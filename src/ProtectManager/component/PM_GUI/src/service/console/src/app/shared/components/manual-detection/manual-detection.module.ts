import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ManualDetectionComponent } from './manual-detection.component';
import { BaseModule } from 'app/shared/base.module';
import { DetectUpperBoundModule } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/detect-upper-bound/detect-upper-bound.module';

@NgModule({
  declarations: [ManualDetectionComponent],
  imports: [CommonModule, BaseModule, DetectUpperBoundModule],
  exports: [ManualDetectionComponent]
})
export class ManualDetectionModule {}

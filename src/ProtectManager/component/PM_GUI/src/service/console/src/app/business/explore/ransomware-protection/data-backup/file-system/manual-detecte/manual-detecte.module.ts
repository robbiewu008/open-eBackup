import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ManualDetecteComponent } from './manual-detecte.component';
import { BaseModule } from 'app/shared';
import { DetectUpperBoundModule } from '../../backup-policy/detect-upper-bound/detect-upper-bound.module';

@NgModule({
  declarations: [ManualDetecteComponent],
  imports: [CommonModule, BaseModule, DetectUpperBoundModule],
  exports: [ManualDetecteComponent]
})
export class ManualDetecteModule {}

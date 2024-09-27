import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectUpperBoundComponent } from './detect-upper-bound.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DetectUpperBoundComponent],
  imports: [CommonModule, BaseModule],
  exports: [DetectUpperBoundComponent]
})
export class DetectUpperBoundModule {}

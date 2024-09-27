import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FeedbackWarningComponent } from './feedback-warning.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [FeedbackWarningComponent],
  imports: [CommonModule, BaseModule],
  exports: [FeedbackWarningComponent]
})
export class FeedbackWarningModule {}

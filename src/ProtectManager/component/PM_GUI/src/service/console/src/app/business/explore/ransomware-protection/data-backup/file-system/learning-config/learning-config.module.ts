import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LearningConfigComponent } from './learning-config.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [LearningConfigComponent],
  imports: [CommonModule, BaseModule],
  exports: [LearningConfigComponent]
})
export class LearningConfigModule {}

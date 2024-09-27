import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CyberSummaryComponent } from './cyber-summary.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CyberSummaryComponent],
  imports: [CommonModule, BaseModule],
  exports: [CyberSummaryComponent]
})
export class CyberSummaryModule {}

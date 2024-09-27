import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryComponent } from './summary.component';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}

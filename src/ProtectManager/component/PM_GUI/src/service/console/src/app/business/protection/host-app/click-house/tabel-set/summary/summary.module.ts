import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SummaryComponent } from './summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, ProTableModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}

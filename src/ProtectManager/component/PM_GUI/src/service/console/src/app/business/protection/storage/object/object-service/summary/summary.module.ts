import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryComponent } from './summary.component';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, ProTableModule, BaseInfoModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}

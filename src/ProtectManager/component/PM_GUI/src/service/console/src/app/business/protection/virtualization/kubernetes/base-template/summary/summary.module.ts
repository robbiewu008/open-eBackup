import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { SummaryComponent } from './summary.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseInfoModule, BaseModule, ProTableModule]
})
export class SummaryModule {}

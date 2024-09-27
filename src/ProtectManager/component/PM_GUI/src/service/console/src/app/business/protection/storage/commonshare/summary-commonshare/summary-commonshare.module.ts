import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { SummaryCommonShareComponent } from './summary-commonshare.component';

@NgModule({
  declarations: [SummaryCommonShareComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule],
  exports: [SummaryCommonShareComponent]
})
export class SummaryCommonShareModule {}

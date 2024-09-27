import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryComponent } from './summary.component';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}

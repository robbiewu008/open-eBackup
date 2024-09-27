import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryComponent } from './summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule]
})
export class SummaryModule {}

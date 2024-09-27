import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';
import { SummaryComponent } from './summary.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    CommonModule,
    ProTableModule,
    BaseInfoModule,
    BaseModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [SummaryComponent]
})
export class SummaryModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryComponent } from './summary.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { BaseTableModule } from '../../virtualization-base/base-table/base-table.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    BaseInfoModule,
    SpecialBaseInfoModule,
    BaseTableModule
  ],
  exports: [SummaryComponent]
})
export class SummaryModule {}

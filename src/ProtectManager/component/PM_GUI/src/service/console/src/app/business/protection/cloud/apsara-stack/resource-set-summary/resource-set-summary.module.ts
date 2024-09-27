import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseTableModule } from 'app/business/protection/virtualization/virtualization-base/base-table/base-table.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SpecialBaseInfoModule } from 'app/shared/components/special-base-info/special-base-info.module';
import { ResourceSetSummaryComponent } from './resource-set-summary.component';

@NgModule({
  declarations: [ResourceSetSummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    SpecialBaseInfoModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    BaseTableModule
  ],
  exports: [ResourceSetSummaryComponent]
})
export class ResourceSetSummaryModule {}

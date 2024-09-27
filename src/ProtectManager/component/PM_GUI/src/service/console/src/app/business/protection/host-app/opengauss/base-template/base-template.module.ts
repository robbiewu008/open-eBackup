import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseTemplateComponent } from './base-template.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { SummaryModule } from './summary/summary.module';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { AddResourceTagModule } from 'app/shared/components/add-resource-tag/add-resource-tag.module';

@NgModule({
  declarations: [BaseTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProButtonModule,
    ProTableModule,
    ProStatusModule,
    SummaryModule,
    BaseInfoModule,
    CopyDataModule,
    AddResourceTagModule
  ],
  exports: [BaseTemplateComponent]
})
export class BaseTemplateModule {}

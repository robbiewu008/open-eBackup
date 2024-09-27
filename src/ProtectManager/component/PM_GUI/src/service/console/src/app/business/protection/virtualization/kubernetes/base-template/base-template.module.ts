import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseTemplateComponent } from './base-template.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { SelectDatabaseListModule } from './select-database-list/select-database-list.module';
import { AddResourceTagModule } from 'app/shared/components/add-resource-tag/add-resource-tag.module';

@NgModule({
  declarations: [BaseTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    SelectDatabaseListModule,
    AddResourceTagModule
  ],
  exports: [BaseTemplateComponent]
})
export class BaseTemplateModule {}

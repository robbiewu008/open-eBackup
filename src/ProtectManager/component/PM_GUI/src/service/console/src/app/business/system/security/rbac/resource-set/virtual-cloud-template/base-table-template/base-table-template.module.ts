import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BaseTableTemplateComponent } from './base-table-template.component';

@NgModule({
  declarations: [BaseTableTemplateComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [BaseTableTemplateComponent]
})
export class BaseTableTemplateModule {}

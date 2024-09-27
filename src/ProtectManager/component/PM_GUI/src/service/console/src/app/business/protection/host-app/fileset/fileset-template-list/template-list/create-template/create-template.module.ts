import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateTemplateComponent } from './create-template.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddPathModule } from './add-path/add-path.module';

@NgModule({
  declarations: [CreateTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    ResourceFilterModule,
    ProTableModule,
    AddPathModule
  ],
  exports: [CreateTemplateComponent]
})
export class CreateTemplateModule {}

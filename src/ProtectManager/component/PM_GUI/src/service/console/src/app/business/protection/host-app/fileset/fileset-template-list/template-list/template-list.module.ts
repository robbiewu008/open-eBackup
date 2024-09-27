import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TemplateListComponent } from './template-list.component';
import { CreateTemplateModule } from './create-template/create-template.module';
import { TemplateDetailModule } from './template-detail/template-detail.module';
import { AssociatedFilesetModule } from './associated-fileset/associated-fileset.module';
import { ModifyTemplateInfoModule } from './modify-template-info/modify-template-info.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [TemplateListComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateTemplateModule,
    TemplateDetailModule,
    AssociatedFilesetModule,
    ModifyTemplateInfoModule,
    CustomTableSearchModule
  ],
  exports: [TemplateListComponent]
})
export class TemplateListModule {}

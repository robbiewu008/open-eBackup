import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { TemplateDetailComponent } from './template-detail.component';
import { AssociatedFilesetModule } from '../associated-fileset/associated-fileset.module';

@NgModule({
  declarations: [TemplateDetailComponent],
  imports: [CommonModule, BaseModule, AssociatedFilesetModule],
  exports: [TemplateDetailComponent]
})
export class TemplateDetailModule {}

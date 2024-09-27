import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyTemplateInfoComponent } from './modify-template-info.component';
import { BaseModule } from 'app/shared';
import { AssociatedFilesetModule } from '../associated-fileset/associated-fileset.module';

@NgModule({
  declarations: [ModifyTemplateInfoComponent],
  imports: [CommonModule, BaseModule, AssociatedFilesetModule],
  exports: [ModifyTemplateInfoComponent]
})
export class ModifyTemplateInfoModule {}

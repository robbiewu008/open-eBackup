import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddLimitComponent } from './add-limit.component';
import { SimpleResourceTemplateModule } from './simple-resource-template/simple-resource-template.module';

@NgModule({
  declarations: [AddLimitComponent],
  imports: [CommonModule, BaseModule, SimpleResourceTemplateModule],
  exports: [AddLimitComponent]
})
export class AddLimitModule {}

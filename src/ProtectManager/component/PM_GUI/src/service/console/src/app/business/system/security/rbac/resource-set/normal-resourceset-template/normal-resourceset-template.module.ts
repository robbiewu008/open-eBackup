import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { NormalResourcesetTemplateComponent } from './normal-resourceset-template.component';

@NgModule({
  declarations: [NormalResourcesetTemplateComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [NormalResourcesetTemplateComponent]
})
export class NormalResourcesetTemplateModule {}

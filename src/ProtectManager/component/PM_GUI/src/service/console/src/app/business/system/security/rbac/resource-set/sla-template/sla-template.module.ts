import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SlaTemplateComponent } from './sla-template.component';

@NgModule({
  declarations: [SlaTemplateComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [SlaTemplateComponent]
})
export class SlaTemplateModule {}

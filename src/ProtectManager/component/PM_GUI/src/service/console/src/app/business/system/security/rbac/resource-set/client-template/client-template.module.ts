import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ClientTemplateComponent } from './client-template.component';

@NgModule({
  declarations: [ClientTemplateComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [ClientTemplateComponent]
})
export class ClientTemplateModule {}

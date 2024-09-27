import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { BaseTemplateModule } from '../base-template/base-template.module';
import { NameDetailModule } from '../base-template/name-detail/name-detail.module';
import { NamespaceComponent } from './namespace.component';

@NgModule({
  declarations: [NamespaceComponent],
  imports: [CommonModule, BaseTemplateModule, NameDetailModule],
  exports: [NamespaceComponent]
})
export class NamespaceModule {}

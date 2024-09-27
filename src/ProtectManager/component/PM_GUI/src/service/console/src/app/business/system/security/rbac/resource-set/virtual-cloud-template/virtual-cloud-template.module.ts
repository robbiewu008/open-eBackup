import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseTableTemplateModule } from './base-table-template/base-table-template.module';
import { VirtualCloudTemplateComponent } from './virtual-cloud-template.component';

@NgModule({
  declarations: [VirtualCloudTemplateComponent],
  imports: [CommonModule, BaseModule, BaseTableTemplateModule],
  exports: [VirtualCloudTemplateComponent]
})
export class VirtualCloudTemplateModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CloudServerComponent } from './cloud-server.component';
import { BaseTemplateModule } from 'app/business/protection/virtualization/kubernetes/base-template/base-template.module';

@NgModule({
  declarations: [CloudServerComponent],
  imports: [CommonModule, BaseModule, BaseTemplateModule],
  exports: [CloudServerComponent]
})
export class CloudServerModule {}

import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { SelectSlaModule } from 'app/shared/components/protect/select-sla/select-sla.module';
import { CreateFilesetModule } from '../../create-fileset/create-fileset.module';
import { CreateFilesetTemplateComponent } from './create-fileset-template.component';
import { TemplateAdvancedParameterModule } from './template-advanced-parameter/template-advanced-parameter.module';
import { BatchResultsModule } from './batch-results/batch-results.module';
import { AdvancedParameterModule } from '../../advanced-parameter/advanced-parameter.module';
@NgModule({
  declarations: [CreateFilesetTemplateComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateFilesetModule,
    SelectSlaModule,
    TemplateAdvancedParameterModule,
    BatchResultsModule,
    AdvancedParameterModule
  ]
})
export class CreateFilesetTemplateModule {}

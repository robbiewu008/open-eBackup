import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseTemplateModule } from '../base-template/base-template.module';
import { SummaryModule } from '../base-template/summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { StatefulsetComponent } from './statefulset.component';

@NgModule({
  declarations: [StatefulsetComponent],
  imports: [CommonModule, BaseTemplateModule, SummaryModule, CopyDataModule],
  exports: [StatefulsetComponent]
})
export class StatefulsetModule {}

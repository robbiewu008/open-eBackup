import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataStatisticalComponent } from './copy-data-statistical.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [CopyDataStatisticalComponent],
  imports: [CommonModule, BaseModule],
  exports: [CopyDataStatisticalComponent]
})
export class CopyDataStatisticalModule {}

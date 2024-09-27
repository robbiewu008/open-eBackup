import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { TakeManualArchiveComponent } from './take-manual-archive.component';

@NgModule({
  declarations: [TakeManualArchiveComponent],
  imports: [CommonModule, BaseModule],
  exports: [TakeManualArchiveComponent]
})
export class TakeManualArchiveModule {}

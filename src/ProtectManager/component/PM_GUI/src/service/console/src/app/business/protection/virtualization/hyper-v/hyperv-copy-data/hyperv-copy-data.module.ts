import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HypervCopyDataComponent } from './hyperv-copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [HypervCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [HypervCopyDataComponent]
})
export class HypervCopyDataModule {}

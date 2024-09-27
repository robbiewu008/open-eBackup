import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';
import { FusionComputeCopyDataComponent } from './fusion-compute-copy-data.component';

@NgModule({
  declarations: [FusionComputeCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [FusionComputeCopyDataComponent]
})
export class FusionComputeCopyDataModule {}

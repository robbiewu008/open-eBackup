import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { KingBaseCopyDataComponent } from './king-base-copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [KingBaseCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [KingBaseCopyDataComponent]
})
export class KingBaseCopyDataModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HCSCopyDataComponent } from './hcs-copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [HCSCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [HCSCopyDataComponent]
})
export class HCSCopyDataModule {}

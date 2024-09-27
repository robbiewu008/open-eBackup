import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataComponent } from './copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  exports: [CopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  declarations: [CopyDataComponent]
})
export class CopyDataModule {}

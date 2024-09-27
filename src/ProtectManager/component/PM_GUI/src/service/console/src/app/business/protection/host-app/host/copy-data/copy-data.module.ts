import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyDataComponent } from './copy-data.component';
import { CopyDataModule as CommonCopyData } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyData],
  exports: [CopyDataComponent]
})
export class CopyDataModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataComponent } from './copy-data.component';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [CommonModule, CommonCopyDataModule],
  exports: [CopyDataComponent]
})
export class CopyDataModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';
import { CopyDataComponent } from './copy-data.component';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [CommonModule, CommonCopyDataModule]
})
export class CopyDataModule {}

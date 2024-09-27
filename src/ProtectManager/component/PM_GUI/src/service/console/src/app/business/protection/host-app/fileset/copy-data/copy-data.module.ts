import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataComponent } from './copy-data.component';
import { CopyDataModule as CommonCopyData } from 'app/shared/components/copy-data/copy-data.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyData]
})
export class CopyDataModule {}

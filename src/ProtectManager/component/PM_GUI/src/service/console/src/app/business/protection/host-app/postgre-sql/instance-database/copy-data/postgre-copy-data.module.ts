import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreCopyDataComponent } from './postgre-copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [PostgreCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [PostgreCopyDataComponent]
})
export class PostgreCopyDataModule {}

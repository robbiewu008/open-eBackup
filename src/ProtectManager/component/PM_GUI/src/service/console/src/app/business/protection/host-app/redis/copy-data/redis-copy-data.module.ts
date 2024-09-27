import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RedisCopyDataComponent } from './redis-copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyDataModule } from 'app/shared/components/copy-data/copy-data.module';

@NgModule({
  declarations: [RedisCopyDataComponent],
  imports: [CommonModule, BaseModule, CommonCopyDataModule],
  exports: [RedisCopyDataComponent]
})
export class RedisCopyDataModule {}

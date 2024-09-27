import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ArchiveStorageDetailComponent } from './archive-storage-detail.component';

@NgModule({
  declarations: [ArchiveStorageDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [ArchiveStorageDetailComponent]
})
export class ArchiveStorageDetailModule {}

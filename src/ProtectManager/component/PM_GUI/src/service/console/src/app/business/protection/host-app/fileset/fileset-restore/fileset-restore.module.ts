import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FilesetRestoreComponent } from './fileset-restore.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';

@NgModule({
  declarations: [FilesetRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule],
  exports: [FilesetRestoreComponent]
})
export class FilesetRestoreModule {}

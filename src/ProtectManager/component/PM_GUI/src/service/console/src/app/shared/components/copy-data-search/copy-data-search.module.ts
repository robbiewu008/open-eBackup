import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FileDetailModule } from 'app/business/search/file-list/file-detail/file-detail.module';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataSearchComponent } from './copy-data-search.component';

@NgModule({
  declarations: [CopyDataSearchComponent],
  imports: [CommonModule, BaseModule, FileDetailModule]
})
export class CopyDataSearchModule {}

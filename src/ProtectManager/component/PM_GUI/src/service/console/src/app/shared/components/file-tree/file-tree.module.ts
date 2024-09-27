import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';
import { FileTreeComponent } from './file-tree.component';
import { BreadcrumbModule } from '@iux/live';

@NgModule({
  declarations: [FileTreeComponent],
  imports: [CommonModule, BaseModule, ProTableModule, BreadcrumbModule],
  exports: [FileTreeComponent]
})
export class FileTreeModule {}

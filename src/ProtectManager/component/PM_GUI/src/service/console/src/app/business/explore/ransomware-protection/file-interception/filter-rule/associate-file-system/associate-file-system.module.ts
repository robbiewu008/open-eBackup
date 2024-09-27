import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AssociateFileSystemComponent } from './associate-file-system.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AssociateFileSystemComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AssociateFileSystemComponent]
})
export class AssociateFileSystemModule {}

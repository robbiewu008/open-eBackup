import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LocalFileSystemRoutingModule } from './local-file-system-routing.module';
import { LocalFileSystemComponent } from './local-file-system.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [LocalFileSystemComponent],
  imports: [
    CommonModule,
    LocalFileSystemRoutingModule,
    BaseModule,
    CopyResourceListModule
  ],
  exports: [LocalFileSystemComponent]
})
export class LocalFileSystemModule {}

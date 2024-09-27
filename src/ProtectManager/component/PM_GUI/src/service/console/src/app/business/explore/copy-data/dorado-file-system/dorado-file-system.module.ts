import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { DoradoFileSystemRoutingModule } from './dorado-file-system-routing.module';
import { DoradoFileSystemComponent } from './dorado-file-system.component';

@NgModule({
  declarations: [DoradoFileSystemComponent],
  imports: [
    CommonModule,
    DoradoFileSystemRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class DoradoFileSystemModule {}

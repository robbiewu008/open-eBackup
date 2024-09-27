import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { FilesetRoutingModule } from './fileset-routing.module';
import { FilesetComponent } from './fileset.component';

@NgModule({
  declarations: [FilesetComponent],
  imports: [
    CommonModule,
    FilesetRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class FilesetModule {}

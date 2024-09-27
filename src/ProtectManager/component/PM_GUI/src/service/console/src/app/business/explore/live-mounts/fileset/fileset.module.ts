import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { FilesetRoutingModule } from './fileset-routing.module';
import { FilesetComponent } from './fileset.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';
@NgModule({
  declarations: [FilesetComponent],
  imports: [
    CommonModule,
    FilesetRoutingModule,
    BaseModule,
    LiveMountsListModule
  ]
})
export class FilesetModule {}

import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterDetailComponent } from './cluster-detail.component';
import { BaseModule } from 'app/shared';
import { FilesetsModule } from '../../filesets/filesets.module';
import { CustomModalOperateModule } from 'app/shared/components';

@NgModule({
  declarations: [ClusterDetailComponent],
  imports: [CommonModule, BaseModule, FilesetsModule, CustomModalOperateModule]
})
export class ClusterDetailModule {}

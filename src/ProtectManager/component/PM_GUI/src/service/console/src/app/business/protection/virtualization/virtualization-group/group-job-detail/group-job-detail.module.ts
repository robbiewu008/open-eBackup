import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { SubTaskResultModule } from '../sub-task-result/sub-task-result.module';
import { GroupJobDetailComponent } from './group-job-detail.component';

@NgModule({
  declarations: [GroupJobDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    SubTaskResultModule,
    CustomModalOperateModule
  ]
})
export class GroupJobDetailModule {}

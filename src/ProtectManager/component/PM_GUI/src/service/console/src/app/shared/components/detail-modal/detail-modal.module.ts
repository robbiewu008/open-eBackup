import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CustomModalOperateModule } from '../custom-modal-operate';
import {
  DetailModalComponent,
  DetailModalOneComponent
} from './detail-modal.component';

@NgModule({
  declarations: [DetailModalComponent, DetailModalOneComponent],
  imports: [CommonModule, BaseModule, CustomModalOperateModule],

  exports: [DetailModalComponent]
})
export class DetailModalModule {}

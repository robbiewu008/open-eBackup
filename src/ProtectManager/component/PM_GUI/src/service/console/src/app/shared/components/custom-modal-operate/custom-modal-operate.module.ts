import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CustomModalOperateComponent } from './custom-modal-operate.component';

@NgModule({
  declarations: [CustomModalOperateComponent],
  imports: [CommonModule, BaseModule],

  exports: [CustomModalOperateComponent]
})
export class CustomModalOperateModule {}

import { CommonModule, DatePipe } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProtectConfigModule } from 'app/shared/consts/protect-config';
import { CreateProtectModalModule } from '../create-protect-modal/create-protect-modal.module';

const modules = [ProtectConfigModule, CreateProtectModalModule];

@NgModule({
  imports: [CommonModule, BaseModule, ...modules]
})
export class ProtectModule {}

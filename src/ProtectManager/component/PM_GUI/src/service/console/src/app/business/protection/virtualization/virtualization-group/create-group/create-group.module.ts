import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateGroupComponent } from './create-group.component';
import { BaseModule } from 'app/shared';
import { TransferModule } from '@iux/live';

@NgModule({
  declarations: [CreateGroupComponent],
  imports: [CommonModule, BaseModule, TransferModule]
})
export class CreateGroupModule {}

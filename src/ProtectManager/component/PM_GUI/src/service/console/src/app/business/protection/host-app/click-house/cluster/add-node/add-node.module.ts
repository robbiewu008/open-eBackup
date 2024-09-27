import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddNodeComponent } from './add-node.component';

@NgModule({
  declarations: [AddNodeComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class AddNodeModule {}

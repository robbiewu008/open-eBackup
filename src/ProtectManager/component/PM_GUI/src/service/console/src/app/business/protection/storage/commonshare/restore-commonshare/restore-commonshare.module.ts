import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LinkModule } from '../link/link.module';
import { RestoreCommonshareComponent } from './restore-commonshare.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [RestoreCommonshareComponent],
  imports: [CommonModule, BaseModule, LinkModule, AlertModule],
  exports: [RestoreCommonshareComponent]
})
export class RestoreCommonshareModule {}

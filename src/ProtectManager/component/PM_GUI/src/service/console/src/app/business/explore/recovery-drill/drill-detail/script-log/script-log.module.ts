import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ScriptLogComponent } from './script-log.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ScriptLogComponent],
  imports: [CommonModule, BaseModule]
})
export class ScriptLogModule {}

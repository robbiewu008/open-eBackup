import { CommonModule } from '@angular/common';
import { Component, Input, NgModule, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { IconModule, TooltipModule } from '@iux/live';
import { I18NService } from 'app/shared/services/i18n.service';
import { RouterUrl } from '../consts/permission.const';
import { AppUtilsService } from '../services/app-utils.service';

@Component({
  selector: 'aui-agent-jumper-tips',
  template: `
    <i
      lv-icon="aui-icon-help"
      [lv-tooltip]="agentTipsTpl"
      lvTooltipTheme="light"
      class="configform-constraint"
      lvColorState="true"
      (mouseenter)="helpHover()"
    ></i>

    <ng-template #agentTipsTpl>
      <span [innerHTML]="agentTips"></span>
    </ng-template>
  `
})
export class AgentsJumperTipsComponent implements OnInit {
  agentTips;

  @Input() type;
  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.agentTips = this.i18n.get('protection_agents_link_tips_label', [
      this.type
    ]);
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.ProtectionHostAppHost);
  }
}

@NgModule({
  imports: [CommonModule, FormsModule, TooltipModule, IconModule],
  declarations: [AgentsJumperTipsComponent],

  exports: [AgentsJumperTipsComponent]
})
export class AgentsJumperTipsModule {}

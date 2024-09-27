import { Injectable } from '@angular/core';
import { ModalService } from '@iux/live';
import { InfoComponent } from '../components/info.component';
import { MODAL_COMMON } from '../consts/live.const';
import { I18NService } from './i18n.service';

interface Options {
  content: string;
  width?: number;
  onOK: () => void;
  onCancel?: () => void;
  lvAfterClose?: (rseult: any) => void;
  noBreak?: boolean;
  header?: string;
}

@Injectable({
  providedIn: 'root'
})
export class InfoMessageService {
  private infoComponent = InfoComponent;

  constructor(
    private drawModalService: ModalService,
    private i18n: I18NService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'infoMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvHeader: options.header || this.i18n.get('common_alarms_info_label'),
        lvContent: this.infoComponent,
        lvComponentParams: {
          content: options.content,
          noBreak: options.noBreak
        },
        lvWidth: options.width || MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvCancel: modal => {
          if (options.onCancel) {
            options.onCancel();
          }
        },
        lvOk: modal => options.onOK(),
        lvAfterClose: result => {
          if (options.lvAfterClose) {
            options.lvAfterClose(result);
          }
        }
      }
    });
  }
}

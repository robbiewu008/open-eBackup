import { Injectable } from '@angular/core';
import { ModalService } from '@iux/live';
import { BackupComponent } from '../components/backup.component';
import { MODAL_COMMON } from '../consts/live.const';
import { I18NService } from './i18n.service';

interface Options {
  content: string;
  onOK: (modal) => void;
  width?: number;
  onCancel?: (modal) => void;
  lvAfterClose?: (result: any) => void;
}

@Injectable({
  providedIn: 'root'
})
export class BackupMessageService {
  private backupComponent = BackupComponent;

  constructor(
    private drawModalService: ModalService,
    private i18n: I18NService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'backupMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvHeader: this.i18n.get('common_alarms_info_label'),
        lvContent: this.backupComponent,
        lvComponentParams: {
          content: options.content
        },
        lvWidth: options.width || MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {},
        lvCancel: modal => {
          if (options.onCancel) {
            options.onCancel(modal);
          }
        },
        lvOk: modal => options.onOK(modal),
        lvAfterClose: result => {
          if (options.lvAfterClose) {
            options.lvAfterClose(result);
          }
        }
      }
    });
  }
}

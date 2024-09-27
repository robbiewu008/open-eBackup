import { CommonModule } from '@angular/common';
import { Component, Injectable, NgModule } from '@angular/core';
import { MessageboxService } from '@iux/live';
import { BaseModule, I18NService } from 'app/shared';
import { Subject, timer } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class BrowserActionService {
  private modal;
  private showZoomTip = localStorage.getItem('IS_SHOW_TIP')
    ? JSON.parse(localStorage.getItem('IS_SHOW_TIP'))
    : true;
  private timerWorkerInited = false;
  private timerWorkerSub$ = new Subject();
  private minInterval = 10;
  private browserActionComponent = BrowserActionComponent;

  constructor(
    private i18n: I18NService,
    private messageBox: MessageboxService
  ) {}

  /**
   * 检测浏览器缩放，当缩放比例不为100%时提示用户
   */
  checkBrowserZoom() {
    // 如果已经提示过用户已忽略并继续则不再检测
    if (!this.showZoomTip) return;

    let isPromptOpen = false;
    let fn = this.interval(() => {
      const ratio = this.getZoom();
      if (ratio !== 100 && !isPromptOpen) {
        this.modal = this.messageBox.info({
          lvModalKey: 'browserZoomKey',
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.browserActionComponent,
          lvOkText: this.i18n.get('common_close_label'),
          lvOk: () => {
            fn();
            fn = null;
          },
          lvAfterOpen: modal => {
            const component = modal.getContentComponent() as BrowserActionComponent;
            component.isChecked$.subscribe(e => {
              localStorage.setItem('IS_SHOW_TIP', JSON.stringify(!e));
              this.showZoomTip = !e;
              isPromptOpen = !e;
            });
          }
        });
        isPromptOpen = true;
      } else if (ratio === 100 && isPromptOpen) {
        this.modal && this.modal.close();
        isPromptOpen = false;
      }
    }, 1000);
  }

  /**
   * 获取缩放比例
   */
  private getZoom() {
    let ratio =
      window.devicePixelRatio !== undefined
        ? window.devicePixelRatio
        : window.outerWidth / window.innerWidth;

    if (ratio) {
      ratio = Math.round(ratio * 100);
    }

    return ratio;
  }

  /**
   * 轮询，可防止浏览器休眠
   * @param fn 执行函数，参数为当前已执行的次数
   * @param time 轮询间隔，必须为minInterval的整数倍，这也意味着有可能有不超过minInterval的延迟
   * @param immediately 是否立即执行一次
   */
  private interval(
    fn: (execTimes: number) => void,
    time: number,
    immediately?: boolean
  ) {
    this.initTimeWorker();

    let count = time / this.minInterval;
    let execCount = 0;
    let sub$ = this.timerWorkerSub$.subscribe(() => {
      count--;
      if (count <= 0) {
        fn(execCount++);
        count = time / this.minInterval;
      }
    });

    const destroy = () => {
      sub$.unsubscribe();
      sub$ = null;
    };

    if (immediately) {
      fn(execCount++);
    }

    return destroy;
  }

  private initTimeWorker() {
    if (this.timerWorkerInited) return;

    try {
      const js = `
                var timer;

                self.onmessage = function (e) {
                    if (e.data == 'start') {
                        sendMessage();
                    } else if (e.data == 'stop') {
                        clearInterval(timer);
                    }
                };

                function sendMessage() {
                    clearInterval(timer);
                    timer = setInterval(function () {
                        self.postMessage('do');
                    }, ${this.minInterval});
                }
            `;
      const blob = new Blob([js], { type: 'application/javascript' });
      const timerWorker = new Worker(URL.createObjectURL(blob));
      timerWorker.onmessage = e => {
        if (e.data === 'do') {
          this.timerWorkerSub$.next(true);
        }
      };
      timerWorker.postMessage('start');
    } catch (e) {
      timer(0, this.minInterval).subscribe(() => {
        this.timerWorkerSub$.next(true);
      });
    }

    this.timerWorkerInited = true;
  }
}

@Component({
  selector: 'aui-browser-action',
  template: `
    <span [innerHTML]="content"></span>
    <div style="margin-top:20px">
      <label
        lv-checkbox
        [(ngModel)]="status"
        (ngModelChange)="showTipConfirmChange($event)"
        >{{ i18n.get("common_don't_show_again_label") }}</label
      >
    </div>
  `,
  styles: []
})
export class BrowserActionComponent {
  status = false;
  content = this.i18n.get('common_browser_zoom_label');
  isChecked$ = new Subject<boolean>();

  constructor(public i18n: I18NService) {}

  showTipConfirmChange(e) {
    this.isChecked$.next(e);
  }
}

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [BrowserActionComponent]
})
export class BrowserActionModule {}

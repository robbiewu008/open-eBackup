import { Component, OnInit } from '@angular/core';
import { CommonConsts, JobAPIService } from 'app/shared';
import { assign, find, isEmpty, isNumber, isString, last } from 'lodash';

@Component({
  selector: 'aui-script-log',
  templateUrl: './script-log.component.html',
  styleUrls: ['./script-log.component.less']
})
export class ScriptLogComponent implements OnInit {
  item;
  isExecuteDetail;
  selectedType = 'pre';
  preScript = '';
  postScript = '';
  logsMap = {};

  constructor(private jobApiService: JobAPIService) {}

  ngOnInit(): void {
    this.getJobLog();
  }

  getJobLog(recordsTemp?, startPage?) {
    if (!this.isExecuteDetail) {
      return;
    }
    const params = {
      jobId: this.item?.jobId,
      orderBy: 'startTime',
      orderType: 'desc',
      startPage: startPage || CommonConsts.PAGE_START,
      pageSize: 200
    };
    this.jobApiService.queryJobLogsUsingGET(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / 200) ||
        res.totalCount === 0
      ) {
        const preLog: string[] =
          find(recordsTemp, {
            logInfo: 'agent_execute_pre_script_success_label'
          })?.logInfoParam ||
          find(recordsTemp, {
            logInfo: 'agent_execute_pre_script_fail_label'
          })?.logInfoParam;
        const postLog: string[] =
          find(recordsTemp, {
            logInfo: 'agent_execute_post_script_success_label'
          })?.logInfoParam ||
          find(recordsTemp, {
            logInfo: 'agent_execute_post_script_fail_label'
          })?.logInfoParam;
        assign(this.logsMap, {
          pre: this.getLogs(last(preLog), true),
          post: this.getLogs(last(postLog))
        });
        return;
      }
      this.getJobLog(recordsTemp, startPage);
    });
  }

  getLogs(logInfo: string, isPre?: boolean): string {
    let logs = '';
    try {
      const param = JSON.parse(logInfo);
      logs = param.MsgEcho?.replace(/\n/g, '<br>');
      if (isPre) {
        this.preScript = param.ScriptName;
      } else {
        this.postScript = param.ScriptName;
      }
    } catch (error) {
      logs = logInfo;
    }
    return logs;
  }
}

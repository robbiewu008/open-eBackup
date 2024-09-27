/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
const fs = require('fs');
const http = require('http');

/**
 * 下载omrp对应的资源信息
 *
 * @param versionId    版本id
 * @param type         导出资源类型，All为导出所有资源、checkIn为导出检入资源
 * @param enSavePath   英文保存路径
 * @param zhSavePath   中文保存路径
 *
 */
const errorVersion = '13691'; //'OceanProtect-DataBackup-1.5.0'
const alarmVersion = '13693'; //'OceanProtect-DataBackup-1.5.0'
const dorado = '12119'; // 'OceanStor-Dorado-6.1.6-OceanProtect-1.5' 12119
const pacific = '9791'; //'OceanStorPacific-8.1.5'
const doradoAlarm = '13537';
const pacificAlarm = '13525';
const retryIndex = 1;
const retryTimes = 20;
const retryInterval = 180 * 1e3;
const downlodResource = (
  versionIds,
  type = 'checkIn',
  enSavePath,
  zhSavePath,
  retryIndex
) => {
  for (const versionId of versionIds) {
    const bodyParams = {
      versionId,
      type
    };

    const options = {
      hostname: 'rnd-omrp.huawei.com', // 测试地址：omrp-beta2.rnd.huawei.com
      path: '/omrp/rest/getAlarmResources',
      method: 'POST'
    };

    const request = http.request(options, res => {
      let data = '';
      res.setEncoding('utf-8');
      res
        .on('data', d => {
          data += d;
        })
        .on('end', () => {
          try {
            const resData = JSON.parse(data);
            if (
              resData.omrp.result !== 'success' ||
              !Array.isArray(resData.omrp.data)
            ) {
              console.error(
                `download resource failed: ${JSON.stringify(resData.omrp)}`
              );
              return;
            }
            console.log('download resource success.');

            const zh_CN = {};
            const en_US = {};
            if ([errorVersion, dorado, pacific].includes(versionId)) {
              resData.omrp.data.forEach(item => {
                item.forEach(item => {
                  zh_CN[item.errorCodeIdDec] = item.reasonSuggest
                    ? [dorado, pacific].includes(versionId)
                      ? item.reasonSuggest
                          .replace(/\n/g, '<br>')
                          .replace(/##[0-9]{2}/g, match => {
                            // dorado错误码替换占位，例：##00 -> {0}
                            return `{${parseInt(match.replace('##', ''))}}`;
                          })
                      : item.reasonSuggest.replace(/\n/g, '<br>')
                    : '--';
                  en_US[item.errorCodeIdDec] = item.enReasonSuggest
                    ? [dorado, pacific].includes(versionId)
                      ? item.enReasonSuggest
                          .replace(/\n/g, '<br>')
                          .replace(/##[0-9]{2}/g, match => {
                            return `{${parseInt(match.replace('##', ''))}}`;
                          })
                      : item.enReasonSuggest.replace(/\n/g, '<br>')
                    : '--';
                });
              });
            } else if (
              [alarmVersion, doradoAlarm, pacificAlarm].includes(versionId)
            ) {
              resData.omrp.data.forEach(item => {
                item.forEach(item => {
                  zh_CN[`${item.alarmId}.alarm.name`] = item.cnEventName
                    ? item.cnEventName.replace(/\n/g, '<br>')
                    : '--';
                  zh_CN[`${item.alarmId}.alarm.advice`] = item.cnRepairSuggest
                    ? item.cnRepairSuggest.replace(/\n/g, '<br>')
                    : '--';
                  zh_CN[`${item.alarmId}.alarm.desc`] = item.cnDesc
                    ? item.cnDesc.replace(/\n/g, '<br>')
                    : '--';
                  zh_CN[`${item.alarmId}.alarm.desc.detail`] = item.cnDesc
                    ? item.cnDesc.replace(/\n/g, '<br>')
                    : '--';
                  zh_CN[`${item.alarmId}.alarm.effect`] = item.cnInfluence
                    ? item.cnInfluence.replace(/\n/g, '<br>')
                    : '--';
                  if ([doradoAlarm, pacificAlarm].includes(versionId)) {
                    zh_CN[
                      `${item.alarmId}.alarm.argument.explain`
                    ] = item.cnArgumentExplain
                      ? item.cnArgumentExplain.replace(/\n/g, '<br>')
                      : '--';
                  }
                  en_US[`${item.alarmId}.alarm.name`] = item.enEventName
                    ? item.enEventName.replace(/\n/g, '<br>')
                    : '--';
                  en_US[`${item.alarmId}.alarm.advice`] = item.enRepairSuggest
                    ? item.enRepairSuggest.replace(/\n/g, '<br>')
                    : '--';
                  en_US[`${item.alarmId}.alarm.desc`] = item.enDesc
                    ? item.enDesc.replace(/\n/g, '<br>')
                    : '--';
                  en_US[`${item.alarmId}.alarm.desc.detail`] = item.enDesc
                    ? item.enDesc.replace(/\n/g, '<br>')
                    : '--';
                  en_US[`${item.alarmId}.alarm.effect`] = item.enInfluence
                    ? item.enInfluence.replace(/\n/g, '<br>')
                    : '--';
                  if ([doradoAlarm, pacificAlarm].includes(versionId)) {
                    en_US[
                      `${item.alarmId}.alarm.argument.explain`
                    ] = item.enArgumentExplain
                      ? item.enArgumentExplain.replace(/\n/g, '<br>')
                      : '--';
                  }
                });
              });
            }

            if (fs.existsSync(zhSavePath)) {
              fs.unlink(zhSavePath, err => {
                fs.writeFileSync(zhSavePath, JSON.stringify(zh_CN));
              });
            } else {
              fs.writeFileSync(zhSavePath, JSON.stringify(zh_CN));
            }

            if (fs.existsSync(enSavePath)) {
              fs.unlink(enSavePath, err => {
                fs.writeFileSync(enSavePath, JSON.stringify(en_US));
              });
            } else {
              fs.writeFileSync(enSavePath, JSON.stringify(en_US));
            }
            console.log(`write i18n resource to ${zhSavePath} success.`);
          } catch (e) {
            console.error(
              `parse failed: ${e.message}, retry ${retryIndex} time(s).`
            );
            retryIndex++;
            if (retryIndex > retryTimes) {
              throw new Error(`write i18n resource to ${zhSavePath} faild.`);
            } else {
              const retryTimeout = setTimeout(() => {
                downlodResource(
                  versionIds,
                  type,
                  enSavePath,
                  zhSavePath,
                  retryIndex
                );
                clearTimeout(retryTimeout);
              }, retryInterval);
            }
          }
        });
    });

    request.setHeader('Content-Type', 'application/json');
    request.setTimeout(300 * 1e3, function() {
      console.error(`request is timeout, retry ${retryIndex} time(s).`);
      request.abort();
      retryIndex++;
      if (retryIndex <= retryTimes) {
        const retryTimeout = setTimeout(() => {
          downlodResource(versionIds, type, enSavePath, zhSavePath, retryIndex);
          clearTimeout(retryTimeout);
        }, retryInterval);
      }
    });
    request.write(JSON.stringify(bodyParams));
    request.end();
  }
};

downlodResource(
  [errorVersion],
  'checkIn',
  './src/assets/i18n/en-us/error-code/common.json',
  './src/assets/i18n/zh-cn/error-code/common.json',
  retryIndex
);

downlodResource(
  [alarmVersion],
  'checkIn',
  './src/assets/i18n/en-us/alarm/common.json',
  './src/assets/i18n/zh-cn/alarm/common.json',
  retryIndex
);

downlodResource(
  [dorado],
  'checkIn',
  './src/assets/i18n/en-us/error-code/dorado_616.json',
  './src/assets/i18n/zh-cn/error-code/dorado_616.json',
  retryIndex
);

downlodResource(
  [pacific],
  'checkIn',
  './src/assets/i18n/en-us/error-code/pacific.json',
  './src/assets/i18n/zh-cn/error-code/pacific.json',
  retryIndex
);

downlodResource(
  [doradoAlarm],
  'checkIn',
  './src/assets/i18n/en-us/alarm/dorado_alarm.json',
  './src/assets/i18n/zh-cn/alarm/dorado_alarm.json',
  retryIndex
);

downlodResource(
  [pacificAlarm],
  'checkIn',
  './src/assets/i18n/en-us/alarm/pacific_alarm.json',
  './src/assets/i18n/zh-cn/alarm/pacific_alarm.json',
  retryIndex
);

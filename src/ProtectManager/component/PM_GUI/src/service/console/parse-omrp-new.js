const fs = require('fs');
const http = require('http');
const https = require('https');
const path = require('path');
const unzip = require('unzip-stream');
const { each, includes, find, assign, isEmpty } = require('lodash');


const opAlarmTemplateId = '155';
const opErrorTemplateId = '160';
const pacificErrorTemplateId = '13';
const doradoErrorTemplateId = '13';
const doradoAlarmTemplateId = '7';
const pacificAlarmTemplateId = '7';
const opAlarmName = 'OceanProtect-DataBackup-1.6.0';
const opErrorName = 'OceanProtect-DataBackup-1.6.0';
const pacificErrorName = 'OceanStorPacific-8.1.5';
const doradoErrorName = 'OceanStor-Dorado-6.1.7-OceanProtect-1.5';
const doradoAlarmName = 'OceanProtect-1.6.0';
const pacificAlarmName = 'OceanStorPacific-8.2.1';

const staticToken = 'ySqt4fMivzibfTh4oouKVeg1q0EGyDw5CJ73BL5L';
const appId = 'com.huawei.ipd.noproduct.tenant_c00377603';
const enterpriseId = '11111111111111111111111111111111'

let opAlarmDocId;
let opErrorDocId;
let pacificErrorDocId;
let doradoErrorDocId;
let doradoAlarmDocId;
let pacificAlarmDocId

const resourcePath = {};

const getIamToken = () => {
  const options = {
    host: 'iam.his-op.huawei.com',
    path: '/iam/auth/token',
    method: 'POST',
    rejectUnauthorized: false
  };
  const params = {
    "data": {
      "attributes": {
        'account': appId,
        'secret': staticToken,
        'project': appId,
        'enterprise': enterpriseId
      },
      "type": "token",
    }
  };
  return new Promise((resolve, reject) => {
    const request = https.request(options, res => {
      let data = '';
      res.setEncoding('utf-8');
      res.on('data', d => {
        data += d;
      }).on('end', () => {
        try {
          const token = JSON.parse(data)?.access_token;
          if (token) {
            resolve(token);
          } else {
            reject(`get error token. pls check url, static token, appId...`);
          }
        } catch (error) {
          reject(error);
        }

      })
    });
    request.setHeader('Content-Type', 'application/json');
    request.write(JSON.stringify(params));
    request.end();
  })
}

const removeDir = (dirParh) => {
  if (fs.existsSync(dirParh)) {
    fs.readdirSync(dirParh).forEach(file => {
      let newPath = path.join(dirParh, file);
      if (fs.lstatSync(newPath).isDirectory()) {
        removeDir(newPath);
      } else {
        fs.unlinkSync(newPath);
      }
    })
    fs.rmdirSync(dirParh);
  }
}

const writeResource = (docId, folderPath) => {
  const zh_CN = {};
  const en_US = {};
  fs.readdirSync(`${folderPath}/omList`).map(file => {
    let filePath = `${folderPath}/omList/${file}`;
    const suffixName = path.extname(file).substring(1);
    if (suffixName === 'json' && !isEmpty(fs.readFileSync(filePath))) {
      let resource
      try {
        resource = JSON.parse(fs.readFileSync(filePath));
      } catch (error) {
        resource = {};
        console.log(`paser json file (${filePath}) failed, cause: ${error}...`);
      }
      each([resource], item => {
        // 告警事件
        if (includes([opAlarmDocId, doradoAlarmDocId, pacificAlarmDocId], docId)) {
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
        } else {
          // 错误码
          zh_CN[item.errorCodeIdDec] = item.reasonSuggest
            ? item.reasonSuggest.replace(/\n/g, '<br>')
            : '--';
          en_US[item.errorCodeIdDec] = item.enReasonSuggest
            ? item.enReasonSuggest.replace(/\n/g, '<br>')
            : '--';
        }
      })
    }
  })
  //写入中文
  if (fs.existsSync(resourcePath[docId]?.zh)) {
    fs.unlink(resourcePath[docId]?.zh, () => {
      fs.writeFileSync(resourcePath[docId]?.zh, JSON.stringify(zh_CN));
    });
  } else {
    fs.writeFileSync(resourcePath[docId]?.zh, JSON.stringify(zh_CN));
  }

  //写入英文
  if (fs.existsSync(resourcePath[docId]?.en)) {
    fs.unlink(resourcePath[docId]?.en, () => {
      fs.writeFileSync(resourcePath[docId]?.en, JSON.stringify(en_US));
    });
  } else {
    fs.writeFileSync(resourcePath[docId]?.en, JSON.stringify(en_US));
  }
  // 删除文件夹
  setTimeout(() => removeDir(folderPath));
  console.log(`write i18n resource to ${resourcePath[docId]?.zh} success.`);
}

const downloadResource = (docId, iamToken) => {
  const options = {
    hostname: 'edm3.huawei.com',
    path: `/edm/projects/${appId}/documents/${docId}`,
    method: 'GET',
    rejectUnauthorized: false
  }
  const req = https.request(options, res => {
    const zipPath = path.join(__dirname, `./omrp_${docId}.zip`);
    let data = '';
    res.setEncoding('binary');
    res.on('data', d => {
      data += d;
    }).on('end', () => {
      try {
        fs.writeFile(zipPath, data, 'binary', () => {
          const folderPath = `./omrp_${docId}`;
          const unzipStream = fs.createReadStream(zipPath).pipe(unzip.Extract({ path: folderPath }));
          unzipStream.on('finish', () => {
            // 删除zip文件
            fs.unlinkSync(zipPath);
            // 写入国际化文件
            setTimeout(() => writeResource(docId, folderPath));
          })
        })
      } catch (error) {
        throw new Error(`download omrp resource failed.`);
      }
    })
  })
  req.setHeader('Authorization', iamToken);
  req.setHeader('X-HIC-Info', appId);
  req.setHeader('Content-Type', 'application/octet-stream');
  req.end();
}

const getResourceDocId = () => {
  const options = {
    hostname: 'rnd-omrp.huawei.com',
    path: '/omrp/rest/push/info',
    method: 'POST',
  }
  return new Promise((resolve, reject) => {
    const req = http.request(options, res => {
      let data = '';
      res.on('data', d => {
        data += d;
      }).on('end', () => {
        try {
          const docIds = JSON.parse(data)?.info;
          opAlarmDocId = find(docIds, item => Number(item.templateId) === Number(opAlarmTemplateId) && item.versionName === opAlarmName)?.docId;
          opErrorDocId = find(docIds, item => Number(item.templateId) === Number(opErrorTemplateId) && item.versionName === opErrorName)?.docId;
          pacificErrorDocId = find(docIds, item => Number(item.templateId) === Number(pacificErrorTemplateId) && item.versionName === pacificErrorName)?.docId;
          doradoErrorDocId = find(docIds, item => Number(item.templateId) === Number(doradoErrorTemplateId) && item.versionName === doradoErrorName)?.docId;
          doradoAlarmDocId = find(docIds, item => Number(item.templateId) === Number(doradoAlarmTemplateId) && item.versionName === doradoAlarmName)?.docId;
          pacificAlarmDocId = find(docIds, item => Number(item.templateId) === Number(pacificAlarmTemplateId) && item.versionName === pacificAlarmName)?.docId;
          if (opAlarmDocId && opErrorDocId && pacificErrorDocId && doradoErrorDocId && doradoAlarmDocId && pacificAlarmDocId) {
            assign(resourcePath, {
              [opAlarmDocId]: {
                zh: './src/assets/i18n/zh-cn/alarm/common.json',
                en: './src/assets/i18n/en-us/alarm/common.json'
              },
              [opErrorDocId]: {
                zh: './src/assets/i18n/zh-cn/error-code/common.json',
                en: './src/assets/i18n/en-us/error-code/common.json'
              },
              [pacificErrorDocId]: {
                zh: './src/assets/i18n/zh-cn/error-code/pacific.json',
                en: './src/assets/i18n/en-us/error-code/pacific.json'
              },
              [doradoErrorDocId]: {
                zh: './src/assets/i18n/zh-cn/error-code/dorado_616.json',
                en: './src/assets/i18n/en-us/error-code/dorado_616.json'
              },
              [doradoAlarmDocId]: {
                zh: './src/assets/i18n/zh-cn/alarm/dorado_alarm.json',
                en: './src/assets/i18n/en-us/alarm/dorado_alarm.json'
              },
              [pacificAlarmDocId]: {
                zh: './src/assets/i18n/zh-cn/alarm/pacific_alarm.json',
                en: './src/assets/i18n/en-us/alarm/pacific_alarm.json'
              }
            });
            resolve([opAlarmDocId, opErrorDocId, pacificErrorDocId, doradoErrorDocId, doradoAlarmDocId, pacificAlarmDocId]);
          } else {
            reject(`opAlarmDocId: ${opAlarmDocId}, opErrorDocId: ${opErrorDocId}, pacificErrorDocId: ${pacificErrorDocId}, doradoErrorDocId:${doradoErrorDocId}, doradoAlarmDocId:${doradoAlarmDocId}, pacificAlarmDocId:${pacificAlarmDocId}`);
          }
        } catch (error) {
          reject(error);
        }
      })
    })
    req.setHeader('Content-Type', 'application/json');
    req.write(JSON.stringify([{
      templateId: opAlarmTemplateId,
      versionName: opAlarmName
    }, {
      templateId: opErrorTemplateId,
      versionName: opErrorName
    }, {
      templateId: pacificErrorTemplateId,
      versionName: pacificErrorName
    }, {
      templateId: doradoErrorTemplateId,
      versionName: doradoErrorName
    }, {
      templateId: doradoAlarmTemplateId,
      versionName: doradoAlarmName
    }, {
      templateId: pacificAlarmTemplateId,
      versionName: pacificAlarmName
    }]));
    req.end();
  })
}

const getOmrpResource = () => {
  getResourceDocId().then(docIds => {
    getIamToken().then(iamToken => {
      each(docIds, docId => {
        downloadResource(docId, iamToken);
      })
    }, error => {
      throw new Error(`get token failed, cause: ${error}`);
    })
  }, error => {
    throw new Error(`get docId failed, cause: ${error}`);
  })
}

getOmrpResource();

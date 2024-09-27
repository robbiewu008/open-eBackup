const express = require('express');
const fs = require('fs');
const path = require('path');
const open = require('open');
const http = require('http');
const { MessageChannel } = require('worker_threads');
const swaggerMiddleware = require('swagger-express-middleware');
const swaggerParserMock = require('swagger-parser-mock');
const Mock = require('mockjs');
const bodyParser = require('body-parser');
const low = require('lowdb');
const object = require('./db/data.json');
const FileSync = require('lowdb/adapters/FileSync');
const adapter = new FileSync('./db/data.json');
const router = express.Router();
const _ = require('lodash');
let app = express();

/*------------------------------配置项-------------------------------*/
let port = 4300;
let swaggerFileName = 'swagger.json';
let swaggerFile = path.join(__dirname, `../swagger/${swaggerFileName}`);
// 参数校验
// 为false时, 不会参数检查.适用于无接口文档时, 先自定义接口
let validateRequest = true;

/*------------------------------------------------------------------*/
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(router);
app.use(express.static(path.join(__dirname, '../dist/pm-gui')));
app.use('/console*', express.static('pm-gui/index.html'));
loadModule(router);

// swagger模拟数据
swaggerMiddleware(swaggerFile, app, function (err, middleware) {
  app.use(
    middleware.metadata(),
    middleware.CORS(),
    middleware.files(),
    middleware.parseRequest(),
    middleware.validateRequest(),
    middleware.mock()
  );

  if (validateRequest) {
    app.use(middleware.parseRequest(), middleware.validateRequest());
  }

  app.listen(port, function () {
    console.log(`The mock server is running at http://localhost:${port}`);
    open('http://localhost:4300/console');
    const specs = swaggerParserMock(
      `http://localhost:${port}/api-docs/${swaggerFileName}`
    );
    specs
      .then(docs => {
        let prefix = docs.basePath;

        Object.keys(docs.paths).forEach(path => {
          let pathCfg = docs.paths[path];
          Object.keys(pathCfg).forEach(method => {
            // 按接口添加路由
            const reg = /:[^\/]+/g,
              url = `${prefix}/rest${path.replace(/\{([^}]*)\}/g, ':$1')}`;
            router[method](url, (req, res) => {
              try {
                const regUrl = '^' + url.replace(reg, '[0-9a-zA-Z-]+') + '$',
                  key = Object.keys(object).find(k =>
                    new RegExp(regUrl).test(k)
                  );
                if (object[key]) {
                  res.json(object[key]);
                } else {
                  let example = JSON.parse(
                    pathCfg[method]['responses']['200'].example
                  );
                  // 利用mockjs模拟数据
                  res.json(Mock.mock(example));
                }
              } catch (e) {
                res.json(e);
              }
            });
          });
        });
      })
      .catch(error => console.error(error));
  });
});

function loadModule() {
  const common = require('../swagger/mock/mock/common.js');
  const db = low(adapter);
  if (common) {
    common(router, db, _)
  }
}

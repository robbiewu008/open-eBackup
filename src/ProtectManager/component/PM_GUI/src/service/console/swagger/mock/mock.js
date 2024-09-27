const express = require('express');
const fs = require('fs');
const path = require('path');
const swaggerMiddleware = require('swagger-express-middleware');
const swaggerParserMock = require('swagger-parser-mock');
const Mock = require('mockjs');
const bodyParser = require('body-parser');
const router = express.Router();
let app = express();

/*------------------------------配置项-------------------------------*/
let port = 8001;
let swaggerFileName = 'swagger.json';
let swaggerFile = path.resolve(__dirname, `../${swaggerFileName}`);
// 参数校验
// 为false时, 不会参数检查.适用于无接口文档时, 先自定义接口
let validateRequest = true;

/*------------------------------------------------------------------*/
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(router);
loadModules(router);

// swagger模拟数据
swaggerMiddleware(swaggerFile, app, function(err, middleware) {
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

  app.listen(port, function() {
    console.log(`The mock server is running at http://localhost:${port}`);

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
              url = prefix + path.replace(/\{([^}]*)\}/g, ':$1');
            router[method](url, (req, res) => {
              try {
                let example = JSON.parse(
                  pathCfg[method]['responses']['200'].example
                );
                // 利用mockjs模拟数据
                res.json(Mock.mock(example));
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

// 加载子模块
function loadModules(router) {
  readdir(__dirname + '/mock');

  function readdir(_path_) {
    fs.readdirSync(_path_).map(file => {
      let filePath = `${_path_}/${file}`;

      if (fs.lstatSync(filePath).isDirectory()) {
        readdir(filePath);
      } else {
        const suffixName = path.extname(file).substring(1);
        if (suffixName !== 'json') {
          let module = require(`${_path_}/${file.slice(0, -3)}`);
          if (module) {
            module(router);
          }
        }
      }
    });
  }
}

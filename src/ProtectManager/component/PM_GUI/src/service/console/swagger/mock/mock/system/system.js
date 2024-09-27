module.exports = function (router, db, _, http) {
  router.get('/console/rest/v1/users', (req, res) => {
    const roleName = _.isString(req.query.filter)
      ? JSON.parse(req.query.filter).roleName
      : '';
    if (roleName) {
      const items = db
        .get('/console/rest/v1/users')
        .get('userList')
        .value()
        .filter(item => {
          return !_.isUndefined(
            _.find(item.rolesSet, { roleName: _.first(roleName) })
          );
        });
      res.send({ total: _.size(items), userList: items });
    } else {
      const items = db
        .get('/console/rest/v1/users')
        .get('userList')
        .filter(item =>
          ['mmdp_admin', 'sysadmin', 'mm_audit'].includes(item.userName)
        )
        .value();
      res.send({ total: _.size(items), userList: items });
    }
  });

  router.post('/rest/v1/auth/action/login', (req, res) => {
    const data = {
      sessionId:
        'userId=88a94c476f12a21e016f12a246e50009-loginTime=16121456597700e4a40920dd097059a71af91a76c030030fd36daba05ae6cba5ea645ec44ce34',
      modifyPassword: false,
      userId: '88a94c476f12a21e016f12a246e50009',
      expireDay: -1
    };

    res.setHeader('Set-Cookie', [
      `_OP_TOKEN_=${_.now()}; domain=localhost; path=/; secure=true;`
    ]);

    res.send(data);
  });
};

module.exports = function (router, db, _) {
    const resourceType = [
        'Fileset',
        'Oracle',
        'VM',
        'Host',
        'Cluster',
        'SQLServer-instance',
        'SQLServer-alwaysOn',
        'SQLServer-database',
        'MySQL-instance',
        'MySQL-database',
        'PostgreInstance',
        'OpenGauss-instance',
        'OpenGauss-database',
        'Dameng-cluster',
        'GaussDBT',
        'DWS-cluster',
        'DWS-database',
        'DWS-schema',
        'DWS-table',
        'Redis',
        'ClickHouse/Cluster',
        'ClickHouse/TableSet',
        'KubernetesNamespace',
        'KubernetesStatefulSet',
        'HDFSFileset',
        'HBaseBackupSet',
        'HiveBackupSet',
        'ElasticSearchBackupSet',
        'NasFileSystem',
        'NasShare',
        'CloudHost',
        'Project'
    ]
    router.get('/console/rest/v1/jobs', (req, res) => {
        let items = []
        if (req.query.jobId) {
            items = db
                .get(`/console/rest/v1/jobs/${req.query.jobId}`).get('records').value()
        } else {
            items = _.filter(db
                .get('/console/rest/v1/jobs').get('records').value(), item => {
                    return _.isEmpty(req.query.statusList) || _.includes(req.query.statusList, item.status)
                })
        }

        res.send({ totalCount: _.size(items), records: items });
    });
    // vmware资源
    router.get('/console/rest/v1/environments', (req, res) => {
        const items = db
            .get(`/console/rest/v1/environments/${JSON.parse(req.query.conditions)?.type}`).get('items').value()
        res.send({
            items: items,
            total: _.size(items),
            pages: 1,
            page_size: 10,
            page_no: 0
        });
    })
    router.get('/console/rest/v1/virtual-resource', (req, res) => {
        let items = []
        if (JSON.parse(req.query.conditions)?.type) {
            items = db
                .get(`/console/rest/v1/virtual-resource/${JSON.parse(req.query.conditions)?.type}`).get('items').value()
        }
        if (JSON.parse(req.query.conditions)?.parent_uuid) {
            items = db
                .get(`/console/rest/v1/virtual-resource/${JSON.parse(req.query.conditions)?.parent_uuid}`).get('items').value()
        }
        res.send({
            items: items,
            total: _.size(items),
            pages: 1,
            page_size: 10,
            page_no: 0
        });
    })
    // oracle资源
    router.get('/console/rest/v1/databases', (req, res) => {
        let items;
        if (JSON.parse(req.query.conditions)?.root_uuid) {
            items = db
                .get(`/console/rest/v1/databases/${JSON.parse(req.query.conditions)?.root_uuid}`).get('items').value()
        } else {
            items = db
                .get(`/console/rest/v1/databases`).get('items').value()
        }
        res.send({
            items: items,
            total: _.size(items),
            pages: 1,
            page_size: 10,
            page_no: 0
        });
    })
    // 副本执行时间
    router.get('/console/rest/v1/protected-objects/:resource_id/backup-time', (req, res) => {
        res.send({ latest_time: "2022-11-08T18:54:19.504811", earliest_time: "2022-11-08T17:00:15.266327", next_time: "2022-11-16T10:49:05" })
    })
    // 资源(V2)
    router.get('/console/rest/v2/resources', (req, res) => {
        let items = [];
        const type = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).type : ''
        const subType = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).subType : [];
        const parentUuid = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).parentUuid : '';
        const rootUuid = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).rootUuid : '';
        const uuid = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).uuid : '';
        const scenario = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).scenario : '';
        const isCluster = _.isString(req.query.conditions) ? JSON.parse(req.query.conditions).isCluster : '';
        if (_.size(_.intersection(subType, ['DBBackupAgent', 'VMBackupAgent', 'UBackupAgent']))) {
            items = db
                .get('/console/rest/v2/resources/host').get('records').value();
        }
        if (_.size(_.intersection(subType, ['Kubernetes']))) {
            items = db
                .get('/console/rest/v2/resources/kubernetes').get('records').value();
        }
        if (_.size(_.intersection(subType, ['KubernetesNamespace']))) {
            items = db
                .get('/console/rest/v2/resources/kubernetesNamespace').get('records').value();
        }
        if (_.size(_.intersection(subType, ['KubernetesStatefulSet']))) {
            items = db
                .get('/console/rest/v2/resources/kubernetesStatefulSet').get('records').value();
        }
        if (_.size(_.intersection(subType, ['Fileset']))) {
            items = db
                .get('/console/rest/v2/resources/Fileset').get('records').value();
        }
        // big data
        if (_.size(_.intersection([subType], ['HDFS']))) {
            items = db
                .get('/console/rest/v2/resources/HDFS').get('records').value();
        }
        if (_.size(_.intersection([subType], ['HDFSFileset']))) {
            items = db
                .get('/console/rest/v2/resources/HDFSFileset').get('records').value();
        }
        if (uuid === '6e9fda76144e42bab77bd3d42004e75a') {
            items = db
                .get('/console/rest/v2/resources/6e9fda76144e42bab77bd3d42004e75a').get('records').value();
        }
        if (_.size(_.intersection([subType], ['HBase']))) {
            items = db
                .get('/console/rest/v2/resources/HBase').get('records').value();
        }
        if (_.size(_.intersection([subType], ['HBaseBackupSet']))) {
            items = db
                .get('/console/rest/v2/resources/HBaseBackupDet').get('records').value();
        }
        if (_.size(_.intersection([subType], ['HBaseBackupSet']))) {
            items = db
                .get('/console/rest/v2/resources/HBaseBackupSet').get('records').value();
        }
        if (_.size(_.intersection([subType], ['Hive']))) {
            items = db
                .get('/console/rest/v2/resources/Hive').get('records').value();
        }
        if (_.size(_.intersection([subType], ['HiveBackupSet']))) {
            items = db
                .get('/console/rest/v2/resources/HiveBackupSet').get('records').value();
        }
        if (_.size(_.intersection([subType], ['ElasticSearch']))) {
            items = db
                .get('/console/rest/v2/resources/ElasticSearch').get('records').value();
        }
        if (_.size(_.intersection([subType], ['ElasticSearchBackupSet']))) {
            items = db
                .get('/console/rest/v2/resources/ElasticSearchBackupSet').get('records').value();
        }
        if (_.size(_.intersection(subType, ['HBaseBackupSetPlugin', 'ElasticSearchBackupSetPlugin', 'HiveBackupSetPlugin', 'FilesetPlugin']))) {
            items = db
                .get('/console/rest/v2/resources/BigDataPlugin').get('records').value();
        }
        // nas
        if (type === 'StorageEquipment' || _.size(_.intersection(subType, ['DoradoV6', 'OceanStorV6', 'OceanStorV5']))) {
            items = db
                .get('/console/rest/v2/resources/StorageEquipment').get('records').value();
        }
        if (_.size(_.intersection(subType, ['NasFileSystem']))) {
            items = JSON.parse(req.query.conditions).parentUuid ? db
                .get('/console/rest/v2/resources/NasFileSystem/recovery').get('records').value() : db
                    .get('/console/rest/v2/resources/NasFileSystem').get('records').value();
        }
        if (_.size(_.intersection(subType, ['NasShare']))) {
            items = db
                .get('/console/rest/v2/resources/NasShare').get('records').value();
        }
        // HCS
        if (type === 'HCS') {
            items = db
                .get('/console/rest/v2/resources/HCS').get('records').value();
        }
        if (type === 'Tenant') {
            items = db
                .get('/console/rest/v2/resources/Tenant').get('records').value();
        }
        if (type === 'Project') {
            items = db
                .get('/console/rest/v2/resources/Project').get('records').value();
        }
        if (type === 'CloudHost') {
            items = db
                .get('/console/rest/v2/resources/CloudHost').get('records').value();
        }
        if (subType === 'HCSContainer') {
            items = db
                .get('/console/rest/v2/resources/HCSContainer').get('records').value();
        }
        // mysql
        if (_.size(_.intersection([subType], ['MySQL-cluster']))) {
            items = db
                .get('/console/rest/v2/resources/MySQL-cluster').get('records').value();
        }
        if (_.size(_.intersection(subType, ['MySQL-instance']))) {
            items = db
                .get('/console/rest/v2/resources/MySQL-instance').get('records').value();
        }
        if (_.size(_.intersection(subType, ['MySQL-database']))) {
            items = db
                .get('/console/rest/v2/resources/MySQL-database').get('records').value();
        }
        // FC
        if ((subType === 'FusionCompute' || _.first(subType) === 'FusionCompute') && type === 'Platform') {
            items = db
                .get('/console/rest/v2/resources/Platform').get('records').value();
        }
        if (subType === 'FusionCompute' && type === 'VM') {
            items = db
                .get('/console/rest/v2/resources/FusionCompute/VM').get('records').value();
        }
        if (subType === 'FusionCompute' && type === 'Host') {
            items = db
                .get('/console/rest/v2/resources/FusionCompute/Host').get('records').value();
        }
        if (subType === 'FusionCompute' && type === 'Cluster') {
            items = db
                .get('/console/rest/v2/resources/FusionCompute/Cluster').get('records').value();
        }
        // FC恢复
        if (rootUuid && _.first(type) === 'Cluster') {
            items = db
                .get(`/console/rest/v2/resources/recovery/Cluster/${rootUuid}`).get('records').value();
        }
        // FC恢复数据存储
        if (uuid && subType === 'FusionCompute') {
            items = db
                .get(`/console/rest/v2/resources/FusionCompute/recovery/datastore/${uuid}`).get('records').value();
        }
        // Redis
        if (_.size(_.intersection(subType, ['Redis']))) {
            items = db
                .get('/console/rest/v2/resources/Redis').get('records').value();
        }
        // postgre
        if (subType === 'PostgreCluster') {
            items = db
                .get('/console/rest/v2/resources/PostgreCluster').get('records').value();
        }
        if (_.size(_.intersection(subType, ['PostgreClusterInstance', 'PostgreInstance']))) {
            items = db
                .get('/console/rest/v2/resources/PostgreInstance').get('records').value();
        }
        // SQLServer
        if (subType === 'SQLServer-cluster') {
            items = db
                .get('/console/rest/v2/resources/SQLServer-cluster').get('records').value();
        }
        if (_.size(_.intersection(subType, ['SQLServer-clusterPlugin']))) {
            items = db
                .get('/console/rest/v2/resources/SQLServer-clusterPlugin').get('records').value();
        }
        if (_.first(subType) === 'UBackupAgent') {
            items = db
                .get('/console/rest/v2/resources/UBackupAgent').get('records').value();
        }
        if (_.size(_.intersection(subType, ['SQLServer-instance', 'SQLServer-clusterInstance']))) {
            items = db
                .get('/console/rest/v2/resources/SQLServer-instance').get('records').value();
        }
        if (subType === 'SQLServer-alwaysOn') {
            items = db
                .get('/console/rest/v2/resources/SQLServer-alwaysOn').get('records').value();
        }
        if (subType === 'SQLServer-database') {
            items = db
                .get('/console/rest/v2/resources/SQLServer-database').get('records').value();
        }
        // sqlserver恢复
        if (type === 'Host' && _.isArray(scenario) && _.isArray(isCluster) && !subType) {
            items = db
                .get('/console/rest/v2/resources/recovery/SQLServer-database').get('records').value();
        }
        // openGauss
        if (subType === 'OpenGauss' || _.first(subType) === 'OpenGauss') {
            items = db
                .get('/console/rest/v2/resources/OpenGauss').get('records').value();
        }
        if (_.size(_.intersection(subType, ['OpenGauss-instance']))) {
            items = db
                .get('/console/rest/v2/resources/OpenGauss-instance').get('records').value();
        }
        if (_.size(_.intersection(subType, ['OpenGauss-database']))) {
            items = db
                .get('/console/rest/v2/resources/OpenGauss-database').get('records').value();
        }
        // dameng
        if (_.size(_.intersection(subType, ['Dameng-cluster', 'Dameng-singleNode']))) {
            items = db
                .get('/console/rest/v2/resources/Dameng-cluster').get('records').value();
        }
        // GaussDBT
        if (_.size(_.intersection(subType, ['GaussDBT']))) {
            items = db
                .get('/console/rest/v2/resources/GaussDBT').get('records').value();
        }
        // GaussDB(DWS)
        if (subType === 'DWS-database') {
            items = db
                .get('/console/rest/v2/resources/DWS-database').get('records').value();
        }
        if (subType === 'DWS-schema') {
            items = db
                .get('/console/rest/v2/resources/DWS-schema').get('records').value();
        }
        if (subType === 'DWS-table') {
            items = db
                .get('/console/rest/v2/resources/DWS-table').get('records').value();
        }
        // GaussDB(DWS)恢复
        if (subType === 'DWS-cluster') {
            items = db
                .get('/console/rest/v1/copies/recovery/DWS-cluster').get('records').value();
        }
        // clickhouse
        if (subType === 'ClickHouse' && type === 'Cluster') {
            items = db
                .get('/console/rest/v2/resources/ClickHouse/Cluster').get('records').value();
        }
        if (subType === 'ClickHouse' && type === 'Database') {
            items = db
                .get('/console/rest/v2/resources/ClickHouse/Database').get('records').value();
        }
        if (subType === 'ClickHouse' && type === 'TableSet') {
            items = db
                .get('/console/rest/v2/resources/ClickHouse/TableSet').get('records').value();
        }
        if (_.size(_.intersection(subType, ['ClickHouse'])) && _.isArray(uuid)) {
            items = db
                .get(`/console/rest/v2/resources/ClickHouse/Cluster/${uuid[1]}`).get('records').value();
        }
        if (_.size(_.intersection(subType, ['ClickHouse'])) && type === 'Cluster') {
            items = db
                .get(`/console/rest/v2/resources/ClickHouse/recovery/Cluster`).get('records').value();
        }
        if (parentUuid && _.isString(parentUuid) && _.first(subType) !== 'NasFileSystem') {
            items = db
                .get(`/console/rest/v2/resources/parentUuid/${parentUuid}`).get('records').value();
        }
        res.send({ totalCount: _.size(items), records: items });
    });
    router.get('/console/rest/v2/environments/:env_id/resources', (req, res) => {
        let item = {};
        if (req.query.resourceType === 'HDFS') {
            item = db
                .get(`/console/rest/v2/environments/HDFS/resources`).value();
        }
        if (req.query.resourceType === 'HBase') {
            item = db
                .get(`/console/rest/v2/environments/HBase/resources`).value();
        }
        if (req.query.resourceType === 'Hive') {
            item = db
                .get(`/console/rest/v2/environments/Hive/resources`).value();
        }
        if (req.query.resourceType === 'Fileset') {
            item = db
                .get(`/console/rest/v2/environments/Fileset/resources`).value();
        }
        if (req.query.resourceType === 'ClickHouse') {
            item = db
                .get(`/console/rest/v2/environments/${req.params.env_id}/resources/${req.query.parentId}`).value();
        }
        res.send(item);
    });
    // 资源详情
    router.get('/console/rest/v2/resources/:resourceId', (req, res) => {
        const item = db
            .get(`/console/rest/v2/resources/${req.params.resourceId}`).value() || {}
        res.send(item);
    });
    // 副本视图年、月
    router.get('/console/rest/v1/copies/statistics', (req, res) => {
        let item = [];
        const date = new Date()
        if (req.query.view === 'year') {
            item.push({
                index: `${req.query.time_point}-${date.getMonth() + 1}`,
                count: 1
            })
        } else {
            item.push({
                index: `${req.query.time_point}-${date.getDate()}`,
                count: 1
            })
        }
        res.send(item);
    })
    // 副本视图时间轴
    router.get('/console/rest/v2/copies/available-time-ranges', (req, res) => {
        if (req.query.resourceId) {
            res.send(db
                .get(`/console/rest/v2/copies/available-time-ranges/${req.query.resourceId}`).value())
        } else {
            res.send({ "totalCount": 0, "records": [] });
        }
    })
    //副本资源
    router.get('/console/rest/v1/copies/summary/resources', (req, res) => {
        let items = []
        if (JSON.parse(req.query.conditions)?.resource_sub_type) {
            items = db
                .get(`/console/rest/v1/copies/summary/resources/${JSON.parse(req.query.conditions)?.resource_sub_type[0]}`).get('items').value()
        }
        if (JSON.parse(req.query.conditions)?.resource_id && !JSON.parse(req.query.conditions)?.resource_sub_type) {
            items = db
                .get(`/console/rest/v1/copies/summary/resources/${JSON.parse(req.query.conditions)?.resource_id}`).get('items').value()
        }
        res.send({
            items: items || [],
            total: _.size(items),
            pages: 1,
            page_size: 10,
            page_no: 0
        });
    })
    //副本
    router.get('/console/rest/v1/copies', (req, res) => {
        let items = []
        if (JSON.parse(req.query.conditions)?.resource_sub_type) {
            items = db
                .get(`/console/rest/v1/copies/${JSON.parse(req.query.conditions)?.resource_sub_type[0]}`).get('items').value()
        }
        if (JSON.parse(req.query.conditions)?.resource_id) {
            items = db
                .get(`/console/rest/v1/copies/${JSON.parse(req.query.conditions)?.resource_id}`).get('items').value()
        }
        res.send({
            items: items || [],
            total: _.size(items),
            pages: 1,
            page_size: 10,
            page_no: 0
        });
    })
    // 任务详情
    router.get('/console/rest/v1/jobs/:jobId/logs', (req, res) => {
        const items = db
            .get(`/console/rest/v1/jobs/${req.params.jobId}/logs`).get('records').value()
        res.send({
            records: items,
            totalCount: _.size(items)
        });
    })
    // check资源接口
    router.post('/console/rest/v2/resources/action/check', (req, res) => {
        res.send([{ "code": 0, "bodyErr": "0", "message": "", "detailParams": null }])
    })
    // 及时挂载
    router.get('/console/rest/v1/live-mount', (req, res) => {
        const subType = JSON.parse(req.query.conditions)?.resource_sub_type
        const items = db
            .get(`/console/rest/v1/live-mount/${_.first(subType)}`).get('items').value()
        res.send({
            items: items,
            page_no: 0,
            page_size: 20,
            pages: 1,
            total: _.size(items)
        })
    })
    // 防勒索
    router.get('/console/rest/v1/copies/detect-statistics', (req, res) => {
        const items = db
            .get(`/console/rest/v1/copies/detect-statistics/${req.query.resource_sub_type}`).get('items').value() || []
        res.send({
            items: items,
            page_no: 0,
            page_size: 20,
            pages: 1,
            total: _.size(items)
        })
    })
    // 全局检索副本
    router.post('/console/rest/v1/search/file', (req, res) => {
        const name = req.body.searchKey
        const items = db
            .get(`/console/rest/v1/search/file`).get('items').value().filter(item => {
                return _.includes(req.body.resourceType, item.resourceType)
            })
        if (name) {
            _.each(items, item => {
                _.extend(item, {
                    name: name,
                    nodeName: name
                })
            })
        }
        res.send({
            items: items,
            page_no: 0,
            page_size: 50,
            pages: 22,
            total: _.size(items)
        })
    })
    // 全局检索资源
    router.get('/console/rest/v1/resource/action/search', (req, res) => {
        const name = JSON.parse(req.query.conditions)?.name
        const items = db
            .get(`/console/rest/v1/resource/action/search`).get('items').value()
        if (name) {
            _.each(items, item => {
                _.extend(item, {
                    name: name
                })
            })
        }
        res.send({
            items: items,
            page_no: 0,
            page_size: 50,
            pages: 22,
            total: _.size(items)
        })
    })
    // 移除保护
    router.delete('/console/rest/v1/protected-objects', (req, res) => {
        // v2接口
        _.each(resourceType, type => {
            if (type === 'Oracle') {
                db.get(`/console/rest/v1/databases`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { sla_compliance: null, sla_id: null, sla_name: null, protection_status: 0 }
                ).write()
            } else if (_.includes(['VM', 'Host', 'Cluster'], type)) {
                db.get(`/console/rest/v1/virtual-resource/${type}`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { sla_compliance: null, sla_id: null, sla_name: null, protection_status: 0 }
                ).write()
            } else {
                if (db.get(`/console/rest/v2/resources/${type}`)?.get('records')) {
                    db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                        { protectedObject: {}, protectionStatus: 0 }
                    ).write()
                }
            }

        })
        res.send({})
    })
    // 添加保护
    router.post('/console/rest/v1/protected-objects/batch', (req, res) => {
        const sla = _.find(db.get('/console/rest/v1/slas').get('items').value(), { uuid: req.body.sla_id })
        _.each(resourceType, type => {
            if (type === 'Oracle') {
                db.get(`/console/rest/v1/databases`).get('items')?.find({ uuid: req.body.resources[0]?.resource_id })?.assign(
                    { sla_compliance: null, sla_id: req.body.sla_id, sla_name: sla?.name, protection_status: 1, ext_parameters: req.body.ext_parameters }
                ).write()
            } else if (_.includes(['VM', 'Host', 'Cluster'], type)) {
                db.get(`/console/rest/v1/virtual-resource/${type}`).get('items')?.find({ uuid: req.body.resources[0]?.resource_id })?.assign(
                    { sla_compliance: null, sla_id: req.body.sla_id, sla_name: sla?.name, protection_status: 1, ext_parameters: req.body.ext_parameters }
                ).write()
            } else {
                if (!_.isEmpty(db.get(`/console/rest/v2/resources/${type}`)?.get('records').value())) {
                    db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resources[0]?.resource_id })?.assign({
                        protectedObject: {
                            extParameters: req.body.ext_parameters,
                            slaCompliance: null,
                            slaId: req.body.sla_id,
                            slaName: sla?.name,
                            status: 1
                        }, protectionStatus: 1
                    }).write()
                }
            }

        })
        res.send({})
    })
    // 修改保护
    router.put('/console/rest/v1/protected-objects', (req, res) => {
        const sla = _.find(db.get('/console/rest/v1/slas').get('items').value(), { uuid: req.body.sla_id })
        _.each(resourceType, type => {
            if (type === 'Oracle') {
                db.get(`/console/rest/v1/databases`).get('items')?.find({ uuid: req.body.resource_id })?.assign(
                    { sla_compliance: null, sla_id: req.body.sla_id, sla_name: sla?.name, protection_status: 1, ext_parameters: req.body.ext_parameters }
                ).write()
            } else if (_.includes(['VM', 'Host', 'Cluster'], type)) {
                db.get(`/console/rest/v1/virtual-resource/${type}`).get('items')?.find({ uuid: req.body.resource_id })?.assign(
                    { sla_compliance: null, sla_id: req.body.sla_id, sla_name: sla?.name, protection_status: 1, ext_parameters: req.body.ext_parameters }
                ).write()
            } else {
                if (!_.isEmpty(db.get(`/console/rest/v2/resources/${type}`)?.get('records').value())) {
                    db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_id })?.assign({
                        protectedObject: {
                            extParameters: req.body.ext_parameters,
                            slaCompliance: null,
                            slaId: req.body.sla_id,
                            slaName: sla?.name,
                            status: 1
                        }, protectionStatus: 1
                    }).write()
                }
            }
        })
        res.send({})
    })
    // 禁用保护
    router.put('/console/rest/v1/protected-objects/status/action/deactivate', (req, res) => {
        _.each(resourceType, type => {
            if (type === 'Oracle') {
                db.get(`/console/rest/v1/databases`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { protection_status: 0, sla_status: false }
                ).write()
            } else if (_.includes(['VM', 'Host', 'Cluster'], type)) {
                db.get(`/console/rest/v1/virtual-resource/${type}`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { protection_status: 0, sla_status: false }
                ).write()
            } else {
                if (db.get(`/console/rest/v2/resources/${type}`)?.get('records')) {
                    const protectedObject = db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_ids[0] }).value()?.protectedObject
                    db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                        { protectionStatus: 0, protectedObject: { ...protectedObject, status: 0 } }
                    ).write()
                }
            }
        })
        res.send({})
    })
    // 激活保护
    router.put('/console/rest/v1/protected-objects/status/action/activate', (req, res) => {
        _.each(resourceType, type => {
            if (type === 'Oracle') {
                db.get(`/console/rest/v1/databases`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { protection_status: 1, sla_status: true }
                ).write()
            } else if (_.includes(['VM', 'Host', 'Cluster'], type)) {
                db.get(`/console/rest/v1/virtual-resource/${type}`).get('items')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                    { protection_status: 1, sla_status: true }
                ).write()
            } else {
                if (db.get(`/console/rest/v2/resources/${type}`)?.get('records')) {
                    const protectedObject = db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_ids[0] }).value()?.protectedObject
                    db.get(`/console/rest/v2/resources/${type}`)?.get('records')?.find({ uuid: req.body.resource_ids[0] })?.assign(
                        { protectionStatus: 1, protectedObject: { ...protectedObject, status: 1 } }
                    ).write()
                }
            }
        })
        res.send({})
    })
};

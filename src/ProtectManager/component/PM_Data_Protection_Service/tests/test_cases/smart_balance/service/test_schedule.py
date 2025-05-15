# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import unittest
from unittest import mock
from app.smart_balance.schemas import ExecuteCluster, ModelName


class TestSchedule(unittest.TestCase):

    def setUp(self):
        # 选1，因为剩余容量大
        nodes = [
            {"esn": 1, "totalCapacity": 15, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 10,
             "runningBackupJobCount": 5, "runningTotalJobCount": 20, "historyBackupJobCount": 100,
             "historySuccessBackupJobCount": 90, "backupType": "3", "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
        },
            {"esn": 2, "totalCapacity": 15, "usedCapacity": 11, "resourceId": 1, "runningTaskSpecCount": 10,
             "runningBackupJobCount": 5, "runningTotalJobCount": 20, "historyBackupJobCount": 100,
             "historySuccessBackupJobCount": 90, "backupType": "3", "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                     '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
             },

        ]
        # 选4，因为历史失败数量少
        nodes_2 = [
            {"esn": 3, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 10,
             "runningBackupJobCount": 5, "runningTotalJobCount": 20, "historyBackupJobCount": 20,
             "historySuccessBackupJobCount": 0, "backupType": "3", "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
            {"esn": 4, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 10,
             "runningBackupJobCount": 5, "runningTotalJobCount": 20, "historyBackupJobCount": 10,
             "historySuccessBackupJobCount": 0, "backupType": "3", "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
        ]
        # 选6，虽然剩余能运行的任务数量少，但是健康度高
        nodes_3 = [
            {"esn": 5, "totalCapacity": 5, "usedCapacity": 2, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 18,
             "historySuccessBackupJobCount": 0, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 10, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
             },
            {"esn": 6, "totalCapacity": 5, "usedCapacity": 2, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 100, "historyBackupJobCount": 100,
             "backupType": "3", "runningBackupJobCount": 10, "flag": "second",
             "unit": '[{"id":"3","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"4","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
             },
        ]
        # 节点7的健康度（历史完成任务率）极低，已用容量多，但是有较多剩余允许运行的线程数，节点8很健康但较少剩余允许运行的线程数，8将被优先分发任务
        nodes_4 = [
            {"esn": 7, "totalCapacity": 5, "usedCapacity": 3, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 0, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 15, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
            {"esn": 8, "totalCapacity": 20, "usedCapacity": 5, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 20,
             "historySuccessBackupJobCount": 100, "historyBackupJobCount": 100,
             "backupType": "3", "runningBackupJobCount": 18, "flag": "second",
             "unit": '[{"id":"3","totalCapacityPool":"2","usedCapacityPool":"2","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"4","totalCapacityPool":"3","usedCapacityPool":"3","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
        ]
        # 节点9的健康度（历史完成任务率）极低，但是有剩余允许运行的线程数，节点10很健康但无剩余允许运行的线程数，9将被优先分发任务
        nodes_5 = [
            {"esn": 9, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 0, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 19, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
            {"esn": 10, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 20,
             "historySuccessBackupJobCount": 100, "historyBackupJobCount": 100,
             "backupType": "3", "runningBackupJobCount": 20, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
        ]

        # 选12
        nodes_6 = [
            {"esn": 11, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 0, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 19, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
            {"esn": 12, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 0, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 0, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                        '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'

             },
        ]
        nodes_7 = [
            {"esn": 11, "totalCapacity": 5, "usedCapacity": 1, "resourceId": 1, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 10, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 19, "flag": "second",
             "unit": '[{"id":"1","totalCapacityPool":"2","usedCapacityPool":"1","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                     '{"id":"2","totalCapacityPool":"3","usedCapacityPool":"1","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
             },
            {"esn": 12, "totalCapacity": 5, "usedCapacity": 2, "resourceId": 2, "runningTaskSpecCount": 20,
             "runningTotalJobCount": 19,
             "historySuccessBackupJobCount": 10, "historyBackupJobCount": 20,
             "backupType": "3", "runningBackupJobCount": 19, "flag": "second",
             "unit": '[{"id":"3","totalCapacityPool":"2","usedCapacityPool":"0","historyBackupJobCountPool":"7","historySuccessBackupJobCountPool":"7","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"},'
                     '{"id":"4","totalCapacityPool":"3","usedCapacityPool":"2","historyBackupJobCountPool":"12","historySuccessBackupJobCountPool":"12","runningTaskSpecCountPool":"10","runningTotalJobCountPool":"9"}]'
             }
        ]
        self.previous_seq_data = [
            [4.0, 1.0, 4.0, 1.0, 1.0, 2.0, 2.0, 4.0, 1.0],
            [0.2, 3, 0.726016, 4, 0.104, 3, 0.7270399999999999, 4, 0.724992, 4, 0.14966, 3,
             0.16329999999999997, 2,
             0.00068359375, 1, 0.7198720000000001, 4],
            [-0.09977, 2, 0.21512799999999999, 3, 0.7989799999999999, 2, 0.215261, 3, 0.419354, 3,
             0.229129, 3, 0.178053,
             3, 0.12859299999999999, 3, 0.236494, 3]
        ]
        self.previous_ml_data = [1.0, 192.946, 7188.48]
        self.nodes = []
        for node in nodes:
            self.nodes.append(ExecuteCluster(**node))
        self.nodes_2 = []
        for node in nodes_2:
            self.nodes_2.append(ExecuteCluster(**node))
        self.nodes_3 = []
        for node in nodes_3:
            self.nodes_3.append(ExecuteCluster(**node))
        self.nodes_4 = []
        for node in nodes_4:
            self.nodes_4.append(ExecuteCluster(**node))
        self.nodes_5 = []
        for node in nodes_5:
            self.nodes_5.append(ExecuteCluster(**node))
        self.nodes_6 = []
        for node in nodes_6:
            self.nodes_6.append(ExecuteCluster(**node))
        self.nodes_7 = []
        for node in nodes_7:
            self.nodes_7.append(ExecuteCluster(**node))

    def test_overweight_policy_for_node_success(self):
        """
        用例名称：测试overweight_policy服务成功
        前置条件：底层成功
        check点：返回长度和预计输出长度一致
        """
        from app.smart_balance.service.schedule import overweight_policy_for_node
        overweight_output = overweight_policy_for_node(self.nodes)
        self.assertEqual([0,1], overweight_output)

        overweight_output = overweight_policy_for_node(self.nodes_4)
        self.assertEqual([1,0], overweight_output)

        overweight_output = overweight_policy_for_node(self.nodes_5)
        self.assertEqual([0,1], overweight_output)

        overweight_output = overweight_policy_for_node(self.nodes_6)
        self.assertEqual([1,0], overweight_output)

    def test_overweight_for_pool_success(self):
        """
        用例名称：测试overweight_for_pool服务成功
        前置条件：底层成功
        check点：返回排序和预计输出一致
        """

        from app.smart_balance.service.schedule import overweight_policy_for_pool

        result = overweight_policy_for_pool(self.nodes_7, sortd_node_index=[1,0])
        self.assertEqual(["3","4","2","1"], result)

        result = overweight_policy_for_pool(self.nodes_4, sortd_node_index=[1,0])
        self.assertEqual(["4","3","2","1"], result)


    @mock.patch('app.smart_balance.service.schedule.online_finetune')
    @mock.patch('app.smart_balance.service.schedule.online_interference')
    @mock.patch('app.smart_balance.service.schedule.read_history_online')
    def test_ai_policy_for_node_success(self, _mock_read_history_online, _mock_online_interference,
                              _mock_online_finetune):
        """
        用例名称：测试aipolicy服务成功
        前置条件：底层成功
        check点：返回排序和预计输出一致
        """
        from app.smart_balance.service.schedule import ai_policy_for_node, ai_policy_for_pool
        _mock_read_history_online.return_value = (self.previous_seq_data, self.previous_ml_data)
        _mock_online_interference.return_value = 1, ModelName.gbrt
        _mock_online_finetune.return_value = 2 / 1024

        result = ai_policy_for_node(self.nodes)[0]
        self.assertEqual([0,1], result)

        _mock_online_finetune.return_value = -1
        result = ai_policy_for_node(self.nodes)[0]
        self.assertEqual([0,1], result)

        _mock_online_finetune.return_value = 2 / 1024
        result = ai_policy_for_node(self.nodes_2)[0]
        self.assertEqual([1,0], result)

        _mock_online_finetune.return_value = 2 / 1024
        result = ai_policy_for_node(self.nodes_3)[0]
        self.assertEqual([1,0], result)

        result = ai_policy_for_pool(self.nodes_3, sortd_node_index=result, predict_capacity=2 / 1024)
        self.assertEqual(["4","3","2","1"], result)





    def test_ai_policy_for_pool_success(self):
        """
        用例名称：测试ai_policy_for_pool服务成功
        前置条件：底层成功
        check点：返回排序和预计输出一致
        """

        from app.smart_balance.service.schedule import ai_policy_for_pool


        result = ai_policy_for_pool(self.nodes_7, sortd_node_index=[1,0], predict_capacity=2 / 1024)
        self.assertEqual(["4","3","2","1"], result)

        result = ai_policy_for_pool(self.nodes_4, sortd_node_index=[1,0], predict_capacity=2 / 1024)
        self.assertEqual(["4","3","2","1"], result)


    @mock.patch('app.smart_balance.selfcorrect.monitor.online_finetune')
    @mock.patch('app.smart_balance.service.schedule.online_interference')
    @mock.patch('app.smart_balance.service.schedule.read_history_online')
    def test_aipolicy_tran_overweight_success(self, _mock_read_history_online, _mock_online_interference,
                                              _mock_online_finetune):
        """
        用例名称：测试不符合aipolicy时候转overweight服务成功
        前置条件：底层成功
        check点：返回排序和预计输出一致
        """
        from app.smart_balance.service.schedule import ai_policy_for_node
        _mock_read_history_online.return_value = (self.previous_seq_data, self.previous_ml_data)
        _mock_online_interference.return_value = 1, "None"
        _mock_online_finetune.return_value = 1

        result = ai_policy_for_node(self.nodes)
        self.assertEqual([0,1], result[0])


    def test_return_random_pool_ids(self):
        """
        用例名称：测试return_random_cluster_ids是否成功
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        from app.smart_balance.service.schedule import return_random_pool_ids
        result = return_random_pool_ids(self.nodes_7)
        self.assertEqual(len(result), 4)

    def test_get_percent(self):
        """
        用例名称：测试get_percent符合所有判断情况
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        from app.smart_balance.service.schedule import get_percent
        args1 = -1
        args2 = 2
        result = get_percent(args1, args2)
        self.assertEqual(result, 2.0)

        args1 = 1
        args2 = 0
        result = get_percent(args1, args2)
        self.assertEqual(result, 2.0)

        args1 = 1
        args2 = 10
        result = get_percent(args1, args2)
        self.assertGreater(result, 1.0)

        args1 = 0
        args2 = 2
        result = get_percent(args1, args2)
        self.assertGreater(result, 0.0)

        args1 = 0
        args2 = 2
        result = get_percent(args1, args2, "corun")
        self.assertEqual(result, 0.0)

        args1 = 3
        args2 = 2
        result = get_percent(args1, args2, "corun")
        self.assertEqual(result, 2.0)

    def test_get_healthy_percent(self):
        """
        用例名称：测试get_healthy_percent符合所有判断情况
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        from app.smart_balance.service.schedule import get_healthy_percent
        args1 = -1
        args2 = 2
        result = get_healthy_percent(args1, args2)
        self.assertEqual(result, 1.0)

        args1 = 0
        args2 = -2
        result = get_healthy_percent(args1, args2)
        self.assertEqual(result, 1.0)

        args1 = 1
        args2 = 0
        result = get_healthy_percent(args1, args2)
        self.assertEqual(result, 1.0)

        args1 = 0
        args2 = 0
        result = get_healthy_percent(args1, args2)
        self.assertGreater(result, 1.0)

        args1 = 0
        args2 = 2
        result = get_healthy_percent(args1, args2)
        self.assertGreater(result, 1.0)

        args1 = 1
        args2 = 2
        result = get_healthy_percent(args1, args2)
        self.assertGreater(result, 0.0)


    @mock.patch('app.smart_balance.service.schedule.ml_predict')
    @mock.patch('app.smart_balance.service.schedule.seq_predict')
    @mock.patch("os.path.isfile")
    def test_online_interference_output(self, _mock_isfile, _mock_seq_predict, _mock_ml_predict):
        """
        用例名称：测试online_interference返回值正确
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        from app.smart_balance.service.schedule import online_interference
        _mock_isfile.side_effect = [True, True, True, True]
        _mock_seq_predict.return_value = 10
        _mock_ml_predict.side_effect = [15, 20, 25]
        result = online_interference(self.previous_seq_data, self.previous_ml_data)
        self.assertEqual(result[0], 15)
        self.assertEqual(result[1], ModelName.gbrt)

        _mock_isfile.side_effect = [True, True, True, True]
        _mock_seq_predict.return_value = -1
        _mock_ml_predict.side_effect = [-1, -1, -1]
        result = online_interference(self.previous_seq_data, self.previous_ml_data)
        self.assertEqual(result[0], -1)
        self.assertEqual(result[1], "None")

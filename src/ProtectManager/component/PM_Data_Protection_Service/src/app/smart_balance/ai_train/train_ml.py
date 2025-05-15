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
import os
import joblib
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor, AdaBoostRegressor, GradientBoostingRegressor
from app.smart_balance.schemas import option, ModelName
from app.common import logger

log = logger.get_logger(__name__)


def save_model(path, pipmodel, model_select):
    """
    :param path: saved path
    :param pipmodel: model need to be saved
    :param model_select: model type
    :return:
    """
    joblib.dump(pipmodel, path)
    log.info(f"save model {model_select} success")


def choose_model(model_select):
    model = None

    if model_select == ModelName.rf:
        model = RandomForestRegressor(criterion=option.rf_criterion, min_samples_leaf=option.rf_min_samples_leaf)

    elif model_select == ModelName.gbrt:
        model = GradientBoostingRegressor()

    elif model_select == ModelName.adabr:
        model = AdaBoostRegressor(n_estimators=option.ada_para)

    return model


def train_ml_model(path, features, labels, model_select=ModelName.gbrt):
    # read local dataset for train model in the sequential form
    os.makedirs(option.save_dir, exist_ok=True)
    features_train, features_test, labels_train, labels_test = train_test_split(features, labels, test_size=0.3)

    model = choose_model(model_select)
    model.fit(features_train.values, labels_train.values)
    score = model.score(features_test.values, labels_test.values)
    log.info(f"{model_select} trained score is: {score}")
    # 保存模型
    save_model(path, model, model_select)

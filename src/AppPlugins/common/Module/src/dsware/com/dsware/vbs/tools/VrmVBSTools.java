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
package com.dsware.vbs.tools;

import java.io.File;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * <Description>
 * 
 */
public class VrmVBSTools
{
    private static final String LOG_CONF_FILE = "conf/log4j.properties";

    private static final Logger LOGGER = Logger.getLogger(VrmVBSTools.class);

    private static final CommandHandler cmdHandler = new CommandHandler();

    /**
     * <private construct>
     */
    private VrmVBSTools()
    {
    }

    /**
     * tools command entry
     * @param args command line input arguments
     */
    public static void main(String[] args)
    {
        initLogConfFile();

        System.exit(VrmVBSTools.executeCommand(args));
    }

    private static void initLogConfFile()
    {
        String logConfigFile = LOG_CONF_FILE;
        File file = new File(logConfigFile);
        if (file.exists())
        {
            PropertyConfigurator.configure(logConfigFile);
        }
    }

    /**
     * <execute command>
     * 
     * @param args cli arguments
     * @return int 0:success, other: fail
     */
    private static int executeCommand(String[] args)
    {
        LOGGER.debug("DswareTool begin execute.");
        int ret = cmdHandler.commandExecute(args);
        System.out.println("ResultCode=" + ret); 
        return ret;
    }
}

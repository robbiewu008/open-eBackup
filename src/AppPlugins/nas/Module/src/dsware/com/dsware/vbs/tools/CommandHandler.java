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

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang3.StringUtils;
import org.apache.log4j.Logger;

/**
 * <Description>
 * 
 */
class CommandHandler
{
    
    private static final String CMD_ARG_KEY_FLAG = "--";
    
    private static final String CMD_ARG_KEY_OPS = "op";
    
    private static final Set<String> CMD_ARG_KEY_NAME_SET = new HashSet<>();
    
    private static final Set<String> CMD_NAME_SET = new HashSet<>();
    
    private static final Logger logger = Logger.getLogger(CommandHandler.class);
    
    static
    {
        CMD_ARG_KEY_NAME_SET.add(CMD_ARG_KEY_OPS);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_DSAIP);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_FULL_COPY_FLAG);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_POOL_ID);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SMART_FLAG);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SNAP_NAME);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SNAP_NAME_DEST);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SNAP_NAME_SRC);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_TARGET_HOST);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_VOL_NAME);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_VOL_SIZE);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_THIN_FLAG);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SNAPNAME_FROM);
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_SNAPNAME_TO);
        //add dswarefloatip
        CMD_ARG_KEY_NAME_SET.add(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
        
        CMD_NAME_SET.add(ToolsContances.CMD_NAME_DELETE_VOLUME);
        CMD_NAME_SET.add(ToolsContances.CMD_NAME_CREATE_BITMAP_VOLUME);
        CMD_NAME_SET.add(ToolsContances.CMD_NAME_QUERY_BITMAP_VOLUME);
        CMD_NAME_SET.add(ToolsContances.CMD_NAME_QUERY_ALL_BITMAP_VOLUME);
        
    }

    int commandExecute(String[] args)
    {
        logger.info("commandExecute start");
        if (null == args || args.length == 0)
        {
            logger.error("Input argument is null");
            return ToolsErrors.TOOLS_ERR_INPUT_NULL;
        }
        
        String cmdOp = getOpertaionName(args);
        if (StringUtils.isBlank(cmdOp))
        {
            logger.error("Lack operation");
            return ToolsErrors.TOOLS_ERR_LACK_OPS;
        }
        
        if (!CMD_NAME_SET.contains(cmdOp))
        {
            logger.error("Command operation is invalid");
            return ToolsErrors.TOOLS_ERR_COMMAND_INVALID;
        }
        
        Map<String, String> cmdArg = commandArgsStandardize(args);
        if (null == cmdArg)
        {
            logger.error("Input argument is invalid");
            return ToolsErrors.TOOLS_ERR_PARAM_INVALID;
        }
        
        String classname = VBSCommand.class.getName();
        String mestmp = "VBSCommand.class.getName" + classname;
        logger.error(mestmp);
        @SuppressWarnings("rawtypes")
        Class c;
        try
        {
            c = Class.forName(VBSCommand.class.getName());
        }
        catch (ClassNotFoundException e)
        {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }

        @SuppressWarnings("rawtypes")
        Class types[] = new Class[1];
 
        try
        {
            types[0] = Class.forName(Map.class.getName());
        }
        catch (ClassNotFoundException e)
        {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }


        @SuppressWarnings("unchecked")
        Method m;
        try
        {
            m = c.getMethod(cmdOp, types);
        } catch (SecurityException | NoSuchMethodException e)
        {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        VBSCommand t = new VBSCommand();


        Object ret;
        try
        {
            ret = m.invoke(t, cmdArg);
        }
        catch (IllegalArgumentException | IllegalAccessException | InvocationTargetException e)
        {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        return (Integer) ret;
        
    }
    
    /**
     * 
     * <command arguments standardize>
     * @param args argument arrays
     * @return Map
     */
    private Map<String, String> commandArgsStandardize(String[] args)
    {
        Map<String, String> cmd = new HashMap<>();
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].startsWith(CMD_ARG_KEY_FLAG))
            {
                String key = args[i].substring(CMD_ARG_KEY_FLAG.length());
                if ((i + 1) < args.length
                        && !args[i + 1].startsWith(CMD_ARG_KEY_FLAG))
                {
                    if (!CMD_ARG_KEY_NAME_SET.contains(key))
                    {
                        printInvalidArgumentError(args, key);
                        return null;
                    }
                    cmd.put(key, args[i + 1]);
                }
                else
                {
                    cmd.put(key, "");
                }
            }
        }
        return cmd;
    }
    
    private String getOpertaionName(String args[])
    {
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].startsWith(CMD_ARG_KEY_FLAG + CMD_ARG_KEY_OPS)
                    && i + 1 < args.length)
            {
                return args[i + 1];
            }
        }
        
        return null;
    }

    private void printInvalidArgumentError(String[] args, String key)
    {
        if (null == key)
        {
            logger.error("Input key is null");
        }
        else
        {
            //String errmsg = "Input argument:" + key;
            String errmsg = "Input argument is invalid.";
            logger.error(errmsg);
        }
        if (null == args || args.length == 0)
        {
            logger.error("Input argument is null");
        }
        else
        {
            //String errmsg = Arrays.toString(args);
            String errmsg = "Input argument is invalid.";
            logger.error(errmsg);
        }
    }
}

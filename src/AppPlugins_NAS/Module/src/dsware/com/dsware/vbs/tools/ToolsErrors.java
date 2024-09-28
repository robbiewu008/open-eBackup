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

/**
 * <Description>
 * 
 */
public final class ToolsErrors
{

    private ToolsErrors()
    {
        
    }
    
    public static final int TOOLS_ERROR = 100000001;
    
    public static final int TOOLS_SUCCESS = 0;
    
    public static final int TOOLS_ERR_INTERAL = 100000000;
    
    public static final int TOOLS_ERR_INPUT_NULL = 100000001;
    
    public static final int TOOLS_ERR_PARAM_INVALID = 100000002;
    
    public static final int TOOLS_ERR_LACK_OPS = 100000003;
    
    public static final int TOOLS_ERR_COMMAND_INVALID = 100000004;
    
    public static final int TOOLS_ERR_COMMAND_FAILED = 100000005;
    
    public static final int TOOLS_ERR_LACK_ARG = 100000006;
}

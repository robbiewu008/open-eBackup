/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : ioctl.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/8
 * Version     : 1.0
 *
 * Description : ioctl字符设备创建、删除函数声明
 *
 */

#ifndef _IOMIRROR_IOCTL_H_
#define _IOMIRROR_IOCTL_H_

int im_ctldev_create(void);
void im_ctldev_remove(void);
int save_bitmap(void);
int save_configfile_data(void);
int reset_bitmap_header(dev_t dev_savebitmap, uint64_t offset);

#endif


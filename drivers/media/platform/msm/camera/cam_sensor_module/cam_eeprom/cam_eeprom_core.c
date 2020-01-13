/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/crc32.h>
#include <media/cam_sensor.h>

#include "cam_eeprom_core.h"
#include "cam_eeprom_soc.h"
#include "cam_debug_util.h"
//HTC_START
#include "cam_sensor_dev.h"
#include "lc898123F40_htc.h"
//HTC_END
//file operation+
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/vmalloc.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
//file operation-

/* --- Arcsoft data --- */
#define MAIN_ARCSOFT_CAL_DATA_SIZE 2016 // 2048 - 32 we need to write 27+2016+5 to let start address of every block be mutiple of 32
#define MAIN_ARCSOFT_HEAD_ADDRESS 0x10A5 // first 27 byte
#define MAIN_ARCSOFT_BODY_ADDRESS 0x10C0 // 32byte per write
#define MAIN_ARCSOFT_TAIL_ADDRESS 0x18A0 // last 5 byte
#define FRONT_ARCSOFT_CAL_DATA_SIZE 2016 // 2048 - 32 we need to write 26+2016+6 to let start address of every block be mutiple of 32
#define FRONT_ARCSOFT_HEAD_ADDRESS 0x1686 // first 26 byte
#define FRONT_ARCSOFT_BODY_ADDRESS 0x16A0 // 32byte per write
#define FRONT_ARCSOFT_TAIL_ADDRESS 0x1E80 // last 6 byte
#define ARCSOFT_EXTRAINFO_SIZE 24
#define MAIN_ARCSOFT_EXTRAINFO_ADDRESS 0x18C0
#define FRONT_ARCSOFT_EXTRAINFO_ADDRESS 0x1E86
/* --- Arcsoft data --- */

// The ST M24C64 EEPROM only allow 'Page Write' to 32 bytes written in a single write cycle
#define I2C_WRITE_SEQ_PAGE_SIZE 32
static void cam_eeprom_write_flash(struct cam_eeprom_ctrl_t *e_ctrl)
{
	struct device_node *of_node = e_ctrl->soc_info.dev->of_node;
	uint32_t cellindex = 5; //initialize as 5
	char calibration_data[8] = {0};
	struct file *pFile = NULL;
	int i = 0;
	int rc = 0;
	int byte_to_write = 0;
	if (0 > of_property_read_u32(of_node, "cell-index", &cellindex)){
		CAM_ERR(CAM_EEPROM, "failed to get cell-index from dsti");
	}
	if (cellindex == 1)
	{
		struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
		i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
		i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
		i2c_reg_settings.size = 8;
		i2c_reg_settings.delay = 0;
		i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
		kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * 8,
		GFP_KERNEL);

		pFile=msm_fopen ("/data/misc/camera/flash_info_Main", O_RDONLY, 0666);
		if (pFile == NULL)
/* HTC_START */
		{
			if (i2c_reg_settings.reg_setting)
				kfree(i2c_reg_settings.reg_setting);
/* HTC_END */
			return ;
/* HTC_START */
		}
/* HTC_END */
		byte_to_write = msm_fread(pFile, 0, calibration_data, 8);
		msm_fclose (pFile);

/* HTC_START */
		if (i2c_reg_settings.reg_setting) {
/* HTC_END */
		for( i = 0 ; i < 8; i++)
		{
			i2c_reg_settings.reg_setting[i].reg_addr = 0x18A5 + i;
			i2c_reg_settings.reg_setting[i].reg_data = calibration_data[i];
			i2c_reg_settings.reg_setting[i].delay = 0;
			i2c_reg_settings.reg_setting[i].data_mask = 0;
			pr_info("[WEEPROM] reg_addr = 0x%X data = 0x%X ",
				i2c_reg_settings.reg_setting[i].reg_addr,
				i2c_reg_settings.reg_setting[i].reg_data);
		}
/* HTC_START */
		}
/* HTC_END */
		rc = camera_io_dev_write_continuous(&e_ctrl->io_master_info,&i2c_reg_settings,1);
		pr_info("[WEEPROM] camera_io_dev_write rc = %d ",rc);
		msleep(10);
		pFile=msm_fopen ("/data/misc/camera/flash_info_Main_Sub", O_RDONLY, 0666);
		if (pFile == NULL)
/* HTC_START */
		{
			if (i2c_reg_settings.reg_setting)
				kfree(i2c_reg_settings.reg_setting);
/* HTC_END */
			return ;
/* HTC_START */
		}
/* HTC_END */

		byte_to_write = msm_fread(pFile, 0, calibration_data, 8);

		msm_fclose (pFile);
/* HTC_START */
		if (i2c_reg_settings.reg_setting) {
/* HTC_END */
		for( i = 0 ; i < 8; i++)
		{
			i2c_reg_settings.reg_setting[i].reg_addr = 0x18AD + i;
			i2c_reg_settings.reg_setting[i].reg_data = calibration_data[i];
			i2c_reg_settings.reg_setting[i].delay = 0;
			i2c_reg_settings.reg_setting[i].data_mask = 0;
			pr_info("[WEEPROM] reg_addr = 0x%X data = 0x%X ",
				i2c_reg_settings.reg_setting[i].reg_addr,
				i2c_reg_settings.reg_setting[i].reg_data);
		}
/* HTC_START */
		}
/* HTC_END */
		rc = camera_io_dev_write_continuous(&e_ctrl->io_master_info,&i2c_reg_settings,1);
		pr_info("[WEEPROM] camera_io_dev_write rc = %d ",rc);

		kfree(i2c_reg_settings.reg_setting);
	}
	msleep(10);
}

/* set_protection on/off are same Address and data, only Slave address is different*/
static void cam_eeprom_set_protection(struct cam_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0;

	struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_settings.size = 1;
	i2c_reg_settings.delay = 0;
	i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
	kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * 1,
	GFP_KERNEL);
/* HTC_START */
	if (i2c_reg_settings.reg_setting) {
/* HTC_END */
	i2c_reg_settings.reg_setting[0].reg_addr = 0x00;
	i2c_reg_settings.reg_setting[0].reg_data = 0x00;
	i2c_reg_settings.reg_setting[0].delay = 0;
/* HTC_START */
	}
/* HTC_END */
	rc = camera_io_dev_write(&e_ctrl->io_master_info,&i2c_reg_settings);
	usleep_range(5000,6000);

	kfree(i2c_reg_settings.reg_setting);

	msleep(10);
}

static int cam_eeprom_write_dual_data(
	struct cam_eeprom_ctrl_t *e_ctrl,
	struct file *pFile,
	int readoffset,
	int readsize,
	int writeaddress,
	struct cam_sensor_i2c_reg_setting *i2c_reg_settings)
{
	// page read
	int rc = 0;
	int i=0;
	char calibration_data[I2C_WRITE_SEQ_PAGE_SIZE] = {0};
	int byte_to_write = 0;

	byte_to_write = msm_fread(pFile, readoffset, calibration_data, readsize);
	if (byte_to_write != readsize) {
		pr_err("%s : read (part %d) error, read %d bytes, read size = %d", __func__, i, byte_to_write,readsize);
		rc = EINVAL;
		return rc;
	}

	for( i = 0 ; i < readsize; i++)
	{
		i2c_reg_settings->reg_setting[i].reg_addr = writeaddress + i;
		i2c_reg_settings->reg_setting[i].reg_data = calibration_data[i];
		i2c_reg_settings->reg_setting[i].delay = 0;
		i2c_reg_settings->reg_setting[i].data_mask = 0;
	}
	i2c_reg_settings->size = readsize;
	rc = camera_io_dev_write_continuous(&e_ctrl->io_master_info,i2c_reg_settings,0);

	return rc;
}

static void cam_eeprom_write_dual_extrainfo_camera(struct cam_eeprom_ctrl_t *e_ctrl)
{
	struct device_node *of_node = e_ctrl->soc_info.dev->of_node;
	uint32_t cellindex = 5; //initialize as 5
	struct file *pFile = NULL;
	struct file *pMFile = NULL;
	struct file *pFFile = NULL;
	int rc = 0;
	if (0 > of_property_read_u32(of_node, "cell-index", &cellindex)){
		CAM_ERR(CAM_EEPROM, "failed to get cell-index from dsti");
	}
	pr_info("[WEEPROM] start write eeprom cellindex %d ",cellindex);
	if (cellindex == 1)
	{
		struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
		i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
		i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
		i2c_reg_settings.size = ARCSOFT_EXTRAINFO_SIZE;
		i2c_reg_settings.delay = 5;
		i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
		kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * ARCSOFT_EXTRAINFO_SIZE,
		GFP_KERNEL);
		pFile = msm_fopen ("/data/misc/camera/Main_System_Arcsoft_data.bin", O_RDONLY, 0666);
		if (pFile == NULL) {
			pr_err("%s : open /data/misc/camera/Main_System_Arcsoft_data.bin fail", __func__);
			rc = EINVAL;
			goto free_mem;
		} else {

			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				0,
				ARCSOFT_EXTRAINFO_SIZE,
				MAIN_ARCSOFT_EXTRAINFO_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (head part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem;
			}

			if ( rc == 0 )
				pMFile = msm_fopen ("/data/misc/camera/dualcam_extrainfo_write_eeprom_success.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
			else
				pMFile = msm_fopen ("/data/misc/camera/dualcam_extrainfo_write_eeprom_fail.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);

			if ( pMFile != NULL ){
				msm_fwrite(pMFile, 0, 0, 0);
				msm_fclose(pMFile);
			}

			msm_fclose(pFile);
		}
free_mem:
		kfree(i2c_reg_settings.reg_setting);
	}
	else if (cellindex == 3)
	{
		struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
		i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
		i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
		i2c_reg_settings.size = ARCSOFT_EXTRAINFO_SIZE;
		i2c_reg_settings.delay = 5;
		i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
		kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * ARCSOFT_EXTRAINFO_SIZE,
		GFP_KERNEL);
		pFile = msm_fopen ("/data/misc/camera/Front_System_Arcsoft_data.bin", O_RDONLY, 0666);
		if (pFile == NULL) {
			pr_err("%s : open /data/misc/camera/Front_System_Arcsoft_data.bin fail", __func__);
			rc = EINVAL;
			goto free_mem2;
		} else {


			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				0,
				ARCSOFT_EXTRAINFO_SIZE,
				FRONT_ARCSOFT_EXTRAINFO_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (head part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem2;
			}

			if ( rc == 0 )
				pFFile = msm_fopen ("/data/misc/camera/dualcam_extrainfo_write_front_eeprom_success.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
			else
				pFFile = msm_fopen ("/data/misc/camera/dualcam_extrainfo_write_front_eeprom_fail.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);

			if ( pFFile != NULL ){
				msm_fwrite(pFFile, 0, 0, 0);
				msm_fclose(pFFile);
			}

			msm_fclose(pFile);
		}
free_mem2:
		kfree(i2c_reg_settings.reg_setting);
	}
	else
	{
		pr_info("[WEEPROM] no need to write dual data in camera cellindex %d",cellindex);
	}
	if(rc == 0)
		pr_info("[WEEPROM] camera cellindex %d write success",cellindex);
	msleep(10);
}

static void cam_eeprom_write_dual_camera(struct cam_eeprom_ctrl_t *e_ctrl)
{
	struct device_node *of_node = e_ctrl->soc_info.dev->of_node;
	uint32_t cellindex = 5; //initialize as 5
	struct file *pFile = NULL;
	struct file *pMFile = NULL;
	struct file *pFFile = NULL;
	int i = 0;
	int rc = 0;
	if (0 > of_property_read_u32(of_node, "cell-index", &cellindex)){
		CAM_ERR(CAM_EEPROM, "failed to get cell-index from dsti");
	}
	pr_info("[WEEPROM] start write eeprom cellindex %d ",cellindex);
	if (cellindex == 1)
	{
		struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
		i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
		i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
		i2c_reg_settings.size = I2C_WRITE_SEQ_PAGE_SIZE;
		i2c_reg_settings.delay = 5;
		i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
		kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * I2C_WRITE_SEQ_PAGE_SIZE,
		GFP_KERNEL);
		pFile = msm_fopen ("/data/misc/camera/Main.bin", O_RDONLY, 0666);
		if (pFile == NULL) {
			pr_err("%s : open /data/misc/camera/Main.bin fail", __func__);
			rc = EINVAL;
			goto free_mem;
		} else {

			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				0,
				27,
				MAIN_ARCSOFT_HEAD_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (head part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem;
			}

			for (i = 0; i < (MAIN_ARCSOFT_CAL_DATA_SIZE/I2C_WRITE_SEQ_PAGE_SIZE); i++)
			{
				rc = cam_eeprom_write_dual_data(
					e_ctrl,pFile,
					27+(i*I2C_WRITE_SEQ_PAGE_SIZE),
					I2C_WRITE_SEQ_PAGE_SIZE,
					MAIN_ARCSOFT_BODY_ADDRESS+(i*I2C_WRITE_SEQ_PAGE_SIZE),
					&i2c_reg_settings);
					if (rc < 0) {
						pr_err("%s : eeprom data write fail (body part %d)\n", __func__, i);
						msm_fclose(pFile);
						goto free_mem;
					}
			}

			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				2043,
				5,
				MAIN_ARCSOFT_TAIL_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (tail part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem;
			}

			if ( rc == 0 )
				pMFile = msm_fopen ("/data/misc/camera/dualcam_write_eeprom_success.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
			else
				pMFile = msm_fopen ("/data/misc/camera/dualcam_write_eeprom_fail.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);

			if ( pMFile != NULL ){
				msm_fwrite(pMFile, 0, 0, 0);
				msm_fclose(pMFile);
			}

			msm_fclose(pFile);
		}
free_mem:
		kfree(i2c_reg_settings.reg_setting);
	}
	else if (cellindex == 3)
	{
		struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
		i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
		i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
		i2c_reg_settings.size = I2C_WRITE_SEQ_PAGE_SIZE;
		i2c_reg_settings.delay = 5;
		i2c_reg_settings.reg_setting = (struct cam_sensor_i2c_reg_array *)
		kzalloc(sizeof(struct cam_sensor_i2c_reg_array) * I2C_WRITE_SEQ_PAGE_SIZE,
		GFP_KERNEL);
		pFile = msm_fopen ("/data/misc/camera/Front.bin", O_RDONLY, 0666);
		if (pFile == NULL) {
			pr_err("%s : open /data/misc/camera/Front.bin fail", __func__);
			rc = EINVAL;
			goto free_mem2;
		} else {


			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				0,
				26,
				FRONT_ARCSOFT_HEAD_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (head part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem2;
			}

			for (i = 0; i < (FRONT_ARCSOFT_CAL_DATA_SIZE/I2C_WRITE_SEQ_PAGE_SIZE); i++)
			{
				rc = cam_eeprom_write_dual_data(
					e_ctrl,pFile,
					26+(i*I2C_WRITE_SEQ_PAGE_SIZE),
					I2C_WRITE_SEQ_PAGE_SIZE,
					FRONT_ARCSOFT_BODY_ADDRESS+(i*I2C_WRITE_SEQ_PAGE_SIZE),
					&i2c_reg_settings);
					if (rc < 0) {
						pr_err("%s : eeprom data write fail (body part %d)\n", __func__, i);
						msm_fclose(pFile);
						goto free_mem2;
					}
			}

			rc = cam_eeprom_write_dual_data(
				e_ctrl,pFile,
				2042,
				6,
				FRONT_ARCSOFT_TAIL_ADDRESS,
				&i2c_reg_settings);
			if (rc < 0) {
				pr_err("%s : eeprom data write fail (tail part)\n", __func__);
				msm_fclose(pFile);
				goto free_mem2;
			}

			if ( rc == 0 )
				pFFile = msm_fopen ("/data/misc/camera/dualcam_write_front_eeprom_success.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);
			else
				pFFile = msm_fopen ("/data/misc/camera/dualcam_write_front_eeprom_fail.txt", O_CREAT|O_RDWR|O_TRUNC, 0666);

			if ( pFFile != NULL ){
				msm_fwrite(pFFile, 0, 0, 0);
				msm_fclose(pFFile);
			}

			msm_fclose(pFile);
		}
free_mem2:
		kfree(i2c_reg_settings.reg_setting);
	}
	else
	{
		pr_info("[WEEPROM] no need to write dual data in camera cellindex %d",cellindex);
	}
	if(rc == 0)
		pr_info("[WEEPROM] camera cellindex %d write success",cellindex);
	msleep(10);
}

/**
 * cam_eeprom_read_memory() - read map data into buffer
 * @e_ctrl:     eeprom control struct
 * @block:      block to be read
 *
 * This function iterates through blocks stored in block->map, reads each
 * region and concatenate them into the pre-allocated block->mapdata
 */
static int cam_eeprom_read_memory(struct cam_eeprom_ctrl_t *e_ctrl,
	struct cam_eeprom_memory_block_t *block)
{
	int                                rc = 0;
	int                                j;
	struct cam_sensor_i2c_reg_setting  i2c_reg_settings;
	struct cam_sensor_i2c_reg_array    i2c_reg_array;
	struct cam_eeprom_memory_map_t    *emap = block->map;
	struct cam_eeprom_soc_private     *eb_info;
	uint8_t                           *memptr = block->mapdata;
//HTC_START
	struct device_node *of_node = e_ctrl->soc_info.dev->of_node;
	uint32_t cellindex = 5; //initialize as 5
	if (0 > of_property_read_u32(of_node, "cell-index", &cellindex)){
		CAM_ERR(CAM_EEPROM, "failed to get cell-index from dsti");
	}
//HTC_END

	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "e_ctrl is NULL");
		return -EINVAL;
	}

	eb_info = (struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;

	for (j = 0; j < block->num_map; j++) {
		CAM_DBG(CAM_EEPROM, "slave-addr = 0x%X", emap[j].saddr);
		if (emap[j].saddr) {
			eb_info->i2c_info.slave_addr = emap[j].saddr;
			rc = cam_eeprom_update_i2c_info(e_ctrl,
				&eb_info->i2c_info);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"failed: to update i2c info rc %d",
					rc);
				return rc;
			}
		}

		if (emap[j].page.valid_size) {
			i2c_reg_settings.addr_type = emap[j].page.addr_type;
			i2c_reg_settings.data_type = emap[j].page.data_type;
			i2c_reg_settings.size = 1;
			i2c_reg_array.reg_addr = emap[j].page.addr;
			i2c_reg_array.reg_data = emap[j].page.data;
			i2c_reg_array.delay = emap[j].page.delay;
			i2c_reg_settings.reg_setting = &i2c_reg_array;
			rc = camera_io_dev_write(&e_ctrl->io_master_info,
				&i2c_reg_settings);
			if (rc) {
				CAM_ERR(CAM_EEPROM, "page write failed rc %d",
					rc);
				return rc;
			}
		}

		if (emap[j].pageen.valid_size) {
			i2c_reg_settings.addr_type = emap[j].pageen.addr_type;
			i2c_reg_settings.data_type = emap[j].pageen.data_type;
			i2c_reg_settings.size = 1;
			i2c_reg_array.reg_addr = emap[j].pageen.addr;
			i2c_reg_array.reg_data = emap[j].pageen.data;
			i2c_reg_array.delay = emap[j].pageen.delay;
			i2c_reg_settings.reg_setting = &i2c_reg_array;
			rc = camera_io_dev_write(&e_ctrl->io_master_info,
				&i2c_reg_settings);
			if (rc) {
				CAM_ERR(CAM_EEPROM, "page enable failed rc %d",
					rc);
				return rc;
			}
		}

		if (emap[j].poll.valid_size) {
			rc = camera_io_dev_poll(&e_ctrl->io_master_info,
				emap[j].poll.addr, emap[j].poll.data,
				0, emap[j].poll.addr_type,
				emap[j].poll.data_type,
				emap[j].poll.delay);
			if (rc) {
				CAM_ERR(CAM_EEPROM, "poll failed rc %d",
					rc);
				return rc;
			}
		}

		if (emap[j].mem.valid_size) {
//HTC_START
			if(cellindex == 0) {
				htc_ext_FlashSectorRead(&e_ctrl->io_master_info, memptr, emap[j].mem.addr, emap[j].mem.valid_size);
			}
			else {
//HTC_END
			rc = camera_io_dev_read_seq(&e_ctrl->io_master_info,
				emap[j].mem.addr, memptr,
				emap[j].mem.addr_type,
				emap[j].mem.data_type,
				emap[j].mem.valid_size);
//HTC_START
			}
//HTC_END
			if (rc) {
				CAM_ERR(CAM_EEPROM, "read failed rc %d",
					rc);
				return rc;
			}
			memptr += emap[j].mem.valid_size;
		}

		if (emap[j].pageen.valid_size) {
			i2c_reg_settings.addr_type = emap[j].pageen.addr_type;
			i2c_reg_settings.data_type = emap[j].pageen.data_type;
			i2c_reg_settings.size = 1;
			i2c_reg_array.reg_addr = emap[j].pageen.addr;
			i2c_reg_array.reg_data = 0;
			i2c_reg_array.delay = emap[j].pageen.delay;
			i2c_reg_settings.reg_setting = &i2c_reg_array;
			rc = camera_io_dev_write(&e_ctrl->io_master_info,
				&i2c_reg_settings);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"page disable failed rc %d",
					rc);
				return rc;
			}
		}
	}
	return rc;
}

/**
 * cam_eeprom_power_up - Power up eeprom hardware
 * @e_ctrl:     ctrl structure
 * @power_info: power up/down info for eeprom
 *
 * Returns success or failure
 */
static int cam_eeprom_power_up(struct cam_eeprom_ctrl_t *e_ctrl,
	struct cam_sensor_power_ctrl_t *power_info)
{
	int32_t                 rc = 0;
	struct cam_hw_soc_info *soc_info =
		&e_ctrl->soc_info;

	/* Parse and fill vreg params for power up settings */
	rc = msm_camera_fill_vreg_params(
		&e_ctrl->soc_info,
		power_info->power_setting,
		power_info->power_setting_size);
	if (rc) {
		CAM_ERR(CAM_EEPROM,
			"failed to fill power up vreg params rc:%d", rc);
		return rc;
	}

	/* Parse and fill vreg params for power down settings*/
	rc = msm_camera_fill_vreg_params(
		&e_ctrl->soc_info,
		power_info->power_down_setting,
		power_info->power_down_setting_size);
	if (rc) {
		CAM_ERR(CAM_EEPROM,
			"failed to fill power down vreg params  rc:%d", rc);
		return rc;
	}

	power_info->dev = soc_info->dev;

	rc = cam_sensor_core_power_up(power_info, soc_info);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed in eeprom power up rc %d", rc);
		return rc;
	}

	if (e_ctrl->io_master_info.master_type == CCI_MASTER) {
		rc = camera_io_init(&(e_ctrl->io_master_info));
		if (rc) {
			CAM_ERR(CAM_EEPROM, "cci_init failed");
			return -EINVAL;
		}
	}
	return rc;
}

/**
 * cam_eeprom_power_down - Power down eeprom hardware
 * @e_ctrl:    ctrl structure
 *
 * Returns success or failure
 */
static int cam_eeprom_power_down(struct cam_eeprom_ctrl_t *e_ctrl)
{
	struct cam_sensor_power_ctrl_t *power_info;
	struct cam_hw_soc_info         *soc_info;
	struct cam_eeprom_soc_private  *soc_private;
	int                             rc = 0;

	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "failed: e_ctrl %pK", e_ctrl);
		return -EINVAL;
	}

	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;
	soc_info = &e_ctrl->soc_info;

	if (!power_info) {
		CAM_ERR(CAM_EEPROM, "failed: power_info %pK", power_info);
		return -EINVAL;
	}
	rc = cam_sensor_util_power_down(power_info, soc_info);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "power down the core is failed:%d", rc);
		return rc;
	}

	if (e_ctrl->io_master_info.master_type == CCI_MASTER)
		camera_io_release(&(e_ctrl->io_master_info));

	return rc;
}

/**
 * cam_eeprom_match_id - match eeprom id
 * @e_ctrl:     ctrl structure
 *
 * Returns success or failure
 */
static int cam_eeprom_match_id(struct cam_eeprom_ctrl_t *e_ctrl)
{
	int                      rc;
	struct camera_io_master *client = &e_ctrl->io_master_info;
	uint8_t                  id[2];

	rc = cam_spi_query_id(client, 0, CAMERA_SENSOR_I2C_TYPE_WORD,
		&id[0], 2);
	if (rc)
		return rc;
	CAM_DBG(CAM_EEPROM, "read 0x%x 0x%x, check 0x%x 0x%x",
		id[0], id[1], client->spi_client->mfr_id0,
		client->spi_client->device_id0);
	if (id[0] != client->spi_client->mfr_id0
		|| id[1] != client->spi_client->device_id0)
		return -ENODEV;
	return 0;
}

/**
 * cam_eeprom_parse_read_memory_map - Parse memory map
 * @of_node:    device node
 * @e_ctrl:     ctrl structure
 *
 * Returns success or failure
 */
int32_t cam_eeprom_parse_read_memory_map(struct device_node *of_node,
	struct cam_eeprom_ctrl_t *e_ctrl)
{
	int32_t                         rc = 0;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_sensor_power_ctrl_t *power_info;

	if (!e_ctrl) {
		CAM_ERR(CAM_EEPROM, "failed: e_ctrl is NULL");
		return -EINVAL;
	}

	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;

	rc = cam_eeprom_parse_dt_memory_map(of_node, &e_ctrl->cal_data);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: eeprom dt parse rc %d", rc);
		return rc;
	}
	rc = cam_eeprom_power_up(e_ctrl, power_info);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "failed: eeprom power up rc %d", rc);
		goto data_mem_free;
	}

	e_ctrl->cam_eeprom_state = CAM_EEPROM_CONFIG;
	if (e_ctrl->eeprom_device_type == MSM_CAMERA_SPI_DEVICE) {
		rc = cam_eeprom_match_id(e_ctrl);
		if (rc) {
			CAM_DBG(CAM_EEPROM, "eeprom not matching %d", rc);
			goto power_down;
		}
	}
	rc = cam_eeprom_read_memory(e_ctrl, &e_ctrl->cal_data);
	if (rc) {
		CAM_ERR(CAM_EEPROM, "read_eeprom_memory failed");
		goto power_down;
	}

	rc = cam_eeprom_power_down(e_ctrl);
	if (rc)
		CAM_ERR(CAM_EEPROM, "failed: eeprom power down rc %d", rc);

	e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
	return rc;
power_down:
	cam_eeprom_power_down(e_ctrl);
data_mem_free:
	vfree(e_ctrl->cal_data.mapdata);
	vfree(e_ctrl->cal_data.map);
	e_ctrl->cal_data.num_data = 0;
	e_ctrl->cal_data.num_map = 0;
	e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
	return rc;
}

/**
 * cam_eeprom_get_dev_handle - get device handle
 * @e_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_get_dev_handle(struct cam_eeprom_ctrl_t *e_ctrl,
	void *arg)
{
	struct cam_sensor_acquire_dev    eeprom_acq_dev;
	struct cam_create_dev_hdl        bridge_params;
	struct cam_control              *cmd = (struct cam_control *)arg;

	if (e_ctrl->bridge_intf.device_hdl != -1) {
		CAM_ERR(CAM_EEPROM, "Device is already acquired");
		return -EFAULT;
	}
	if (copy_from_user(&eeprom_acq_dev, (void __user *) cmd->handle,
		sizeof(eeprom_acq_dev))) {
		CAM_ERR(CAM_EEPROM,
			"EEPROM:ACQUIRE_DEV: copy from user failed");
		return -EFAULT;
	}

	bridge_params.session_hdl = eeprom_acq_dev.session_handle;
	bridge_params.ops = &e_ctrl->bridge_intf.ops;
	bridge_params.v4l2_sub_dev_flag = 0;
	bridge_params.media_entity_flag = 0;
	bridge_params.priv = e_ctrl;

	eeprom_acq_dev.device_handle =
		cam_create_device_hdl(&bridge_params);
	e_ctrl->bridge_intf.device_hdl = eeprom_acq_dev.device_handle;
	e_ctrl->bridge_intf.session_hdl = eeprom_acq_dev.session_handle;

	CAM_DBG(CAM_EEPROM, "Device Handle: %d", eeprom_acq_dev.device_handle);
	if (copy_to_user((void __user *) cmd->handle, &eeprom_acq_dev,
		sizeof(struct cam_sensor_acquire_dev))) {
		CAM_ERR(CAM_EEPROM, "EEPROM:ACQUIRE_DEV: copy to user failed");
		return -EFAULT;
	}
	return 0;
}

/**
 * cam_eeprom_update_slaveInfo - Update slave info
 * @e_ctrl:     ctrl structure
 * @cmd_buf:    command buffer
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_update_slaveInfo(struct cam_eeprom_ctrl_t *e_ctrl,
	void *cmd_buf)
{
	int32_t                         rc = 0;
	struct cam_eeprom_soc_private  *soc_private;
	struct cam_cmd_i2c_info        *cmd_i2c_info = NULL;

	soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	cmd_i2c_info = (struct cam_cmd_i2c_info *)cmd_buf;
	soc_private->i2c_info.slave_addr = cmd_i2c_info->slave_addr;
	soc_private->i2c_info.i2c_freq_mode = cmd_i2c_info->i2c_freq_mode;

	rc = cam_eeprom_update_i2c_info(e_ctrl,
		&soc_private->i2c_info);
	CAM_DBG(CAM_EEPROM, "Slave addr: 0x%x Freq Mode: %d",
		soc_private->i2c_info.slave_addr,
		soc_private->i2c_info.i2c_freq_mode);

	return rc;
}

/**
 * cam_eeprom_parse_memory_map - Parse memory map info
 * @data:             memory block data
 * @cmd_buf:          command buffer
 * @cmd_length:       command buffer length
 * @num_map:          memory map size
 * @cmd_length_bytes: command length processed in this function
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_parse_memory_map(
	struct cam_eeprom_memory_block_t *data,
	void *cmd_buf, int cmd_length, uint16_t *cmd_length_bytes,
	int *num_map)
{
	int32_t                            rc = 0;
	int32_t                            cnt = 0;
	int32_t                            processed_size = 0;
	uint8_t                            generic_op_code;
	struct cam_eeprom_memory_map_t    *map = data->map;
	struct common_header              *cmm_hdr =
		(struct common_header *)cmd_buf;
	uint16_t                           cmd_length_in_bytes = 0;
	struct cam_cmd_i2c_random_wr      *i2c_random_wr = NULL;
	struct cam_cmd_i2c_continuous_rd  *i2c_cont_rd = NULL;
	struct cam_cmd_conditional_wait   *i2c_poll = NULL;
	struct cam_cmd_unconditional_wait *i2c_uncond_wait = NULL;

	generic_op_code = cmm_hdr->third_byte;
	switch (cmm_hdr->cmd_type) {
	case CAMERA_SENSOR_CMD_TYPE_I2C_RNDM_WR:
		i2c_random_wr = (struct cam_cmd_i2c_random_wr *)cmd_buf;
		cmd_length_in_bytes   = sizeof(struct cam_cmd_i2c_random_wr) +
			((i2c_random_wr->header.count - 1) *
			sizeof(struct i2c_random_wr_payload));

		for (cnt = 0; cnt < (i2c_random_wr->header.count);
			cnt++) {
			map[*num_map + cnt].page.addr =
				i2c_random_wr->random_wr_payload[cnt].reg_addr;
			map[*num_map + cnt].page.addr_type =
				i2c_random_wr->header.addr_type;
			map[*num_map + cnt].page.data =
				i2c_random_wr->random_wr_payload[cnt].reg_data;
			map[*num_map + cnt].page.data_type =
				i2c_random_wr->header.data_type;
			map[*num_map + cnt].page.valid_size = 1;
		}

		*num_map += (i2c_random_wr->header.count - 1);
		cmd_buf += cmd_length_in_bytes / sizeof(int32_t);
		processed_size +=
			cmd_length_in_bytes;
		break;
	case CAMERA_SENSOR_CMD_TYPE_I2C_CONT_RD:
		i2c_cont_rd = (struct cam_cmd_i2c_continuous_rd *)cmd_buf;
		cmd_length_in_bytes = sizeof(struct cam_cmd_i2c_continuous_rd);

		map[*num_map].mem.addr = i2c_cont_rd->reg_addr;
		map[*num_map].mem.addr_type = i2c_cont_rd->header.addr_type;
		map[*num_map].mem.data_type = i2c_cont_rd->header.data_type;
		map[*num_map].mem.valid_size =
			i2c_cont_rd->header.count;
		cmd_buf += cmd_length_in_bytes / sizeof(int32_t);
		processed_size +=
			cmd_length_in_bytes;
		data->num_data += map[*num_map].mem.valid_size;
		break;
	case CAMERA_SENSOR_CMD_TYPE_WAIT:
		if (generic_op_code ==
			CAMERA_SENSOR_WAIT_OP_HW_UCND ||
			generic_op_code ==
			CAMERA_SENSOR_WAIT_OP_SW_UCND) {
			i2c_uncond_wait =
				(struct cam_cmd_unconditional_wait *)cmd_buf;
			cmd_length_in_bytes =
				sizeof(struct cam_cmd_unconditional_wait);

			if (*num_map < 1) {
				CAM_ERR(CAM_EEPROM,
					"invalid map number, num_map=%d",
					*num_map);
				return -EINVAL;
			}

			/*
			 * Though delay is added all of them, but delay will
			 * be applicable to only one of them as only one of
			 * them will have valid_size set to >= 1.
			 */
			map[*num_map - 1].mem.delay = i2c_uncond_wait->delay;
			map[*num_map - 1].page.delay = i2c_uncond_wait->delay;
			map[*num_map - 1].pageen.delay = i2c_uncond_wait->delay;
		} else if (generic_op_code ==
			CAMERA_SENSOR_WAIT_OP_COND) {
			i2c_poll = (struct cam_cmd_conditional_wait *)cmd_buf;
			cmd_length_in_bytes =
				sizeof(struct cam_cmd_conditional_wait);

			map[*num_map].poll.addr = i2c_poll->reg_addr;
			map[*num_map].poll.addr_type = i2c_poll->addr_type;
			map[*num_map].poll.data = i2c_poll->reg_data;
			map[*num_map].poll.data_type = i2c_poll->data_type;
			map[*num_map].poll.delay = i2c_poll->timeout;
			map[*num_map].poll.valid_size = 1;
		}
		cmd_buf += cmd_length_in_bytes / sizeof(int32_t);
		processed_size +=
			cmd_length_in_bytes;
		break;
	default:
		break;
	}

	*cmd_length_bytes = processed_size;
	return rc;
}

/**
 * cam_eeprom_init_pkt_parser - Parse eeprom packet
 * @e_ctrl:       ctrl structure
 * @csl_packet:	  csl packet received
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_init_pkt_parser(struct cam_eeprom_ctrl_t *e_ctrl,
	struct cam_packet *csl_packet)
{
	int32_t                         rc = 0;
	int                             i = 0;
	struct cam_cmd_buf_desc        *cmd_desc = NULL;
	uint32_t                       *offset = NULL;
	uint32_t                       *cmd_buf = NULL;
	uint64_t                        generic_pkt_addr;
	size_t                          pkt_len = 0;
	uint32_t                        total_cmd_buf_in_bytes = 0;
	uint32_t                        processed_cmd_buf_in_bytes = 0;
	struct common_header           *cmm_hdr = NULL;
	uint16_t                        cmd_length_in_bytes = 0;
	struct cam_cmd_i2c_info        *i2c_info = NULL;
	int                             num_map = -1;
	struct cam_eeprom_memory_map_t *map = NULL;
	struct cam_eeprom_soc_private  *soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;

	e_ctrl->cal_data.map = vzalloc((MSM_EEPROM_MEMORY_MAP_MAX_SIZE *
		MSM_EEPROM_MAX_MEM_MAP_CNT) *
		(sizeof(struct cam_eeprom_memory_map_t)));
	if (!e_ctrl->cal_data.map) {
		rc = -ENOMEM;
		CAM_ERR(CAM_EEPROM, "failed");
		return rc;
	}
	map = e_ctrl->cal_data.map;

	offset = (uint32_t *)&csl_packet->payload;
	offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
	cmd_desc = (struct cam_cmd_buf_desc *)(offset);

	/* Loop through multiple command buffers */
	for (i = 0; i < csl_packet->num_cmd_buf; i++) {
		total_cmd_buf_in_bytes = cmd_desc[i].length;
		processed_cmd_buf_in_bytes = 0;
		if (!total_cmd_buf_in_bytes)
			continue;
		rc = cam_mem_get_cpu_buf(cmd_desc[i].mem_handle,
			(uint64_t *)&generic_pkt_addr, &pkt_len);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "Failed to get cpu buf");
			return rc;
		}
		cmd_buf = (uint32_t *)generic_pkt_addr;
		if (!cmd_buf) {
			CAM_ERR(CAM_EEPROM, "invalid cmd buf");
			return -EINVAL;
		}
		cmd_buf += cmd_desc[i].offset / sizeof(uint32_t);
		/* Loop through multiple cmd formats in one cmd buffer */
		while (processed_cmd_buf_in_bytes < total_cmd_buf_in_bytes) {
			cmm_hdr = (struct common_header *)cmd_buf;
			switch (cmm_hdr->cmd_type) {
			case CAMERA_SENSOR_CMD_TYPE_I2C_INFO:
				i2c_info = (struct cam_cmd_i2c_info *)cmd_buf;
				/* Configure the following map slave address */
				map[num_map + 1].saddr = i2c_info->slave_addr;
				rc = cam_eeprom_update_slaveInfo(e_ctrl,
					cmd_buf);
				cmd_length_in_bytes =
					sizeof(struct cam_cmd_i2c_info);
				processed_cmd_buf_in_bytes +=
					cmd_length_in_bytes;
				cmd_buf += cmd_length_in_bytes/
					sizeof(uint32_t);
				break;
			case CAMERA_SENSOR_CMD_TYPE_PWR_UP:
			case CAMERA_SENSOR_CMD_TYPE_PWR_DOWN:
				cmd_length_in_bytes = total_cmd_buf_in_bytes;
				rc = cam_sensor_update_power_settings(cmd_buf,
					cmd_length_in_bytes, power_info);
				processed_cmd_buf_in_bytes +=
					cmd_length_in_bytes;
				cmd_buf += cmd_length_in_bytes/
					sizeof(uint32_t);
				if (rc) {
					CAM_ERR(CAM_EEPROM, "Failed");
					return rc;
				}
				break;
			case CAMERA_SENSOR_CMD_TYPE_I2C_RNDM_WR:
			case CAMERA_SENSOR_CMD_TYPE_I2C_CONT_RD:
			case CAMERA_SENSOR_CMD_TYPE_WAIT:
				num_map++;
				rc = cam_eeprom_parse_memory_map(
					&e_ctrl->cal_data, cmd_buf,
					total_cmd_buf_in_bytes,
					&cmd_length_in_bytes, &num_map);
				processed_cmd_buf_in_bytes +=
					cmd_length_in_bytes;
				cmd_buf += cmd_length_in_bytes/sizeof(uint32_t);
				break;
			default:
				break;
			}
		}
		e_ctrl->cal_data.num_map = num_map + 1;
	}
	return rc;
}

/**
 * cam_eeprom_get_cal_data - parse the userspace IO config and
 *                                        copy read data to share with userspace
 * @e_ctrl:     ctrl structure
 * @csl_packet: csl packet received
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_get_cal_data(struct cam_eeprom_ctrl_t *e_ctrl,
	struct cam_packet *csl_packet)
{
	struct cam_buf_io_cfg *io_cfg;
	uint32_t              i = 0;
	int                   rc = 0;
	uint64_t              buf_addr;
	size_t                buf_size;
	uint8_t               *read_buffer;

	io_cfg = (struct cam_buf_io_cfg *) ((uint8_t *)
		&csl_packet->payload +
		csl_packet->io_configs_offset);

	CAM_DBG(CAM_EEPROM, "number of IO configs: %d:",
		csl_packet->num_io_configs);

	for (i = 0; i < csl_packet->num_io_configs; i++) {
		CAM_DBG(CAM_EEPROM, "Direction: %d:", io_cfg->direction);
		if (io_cfg->direction == CAM_BUF_OUTPUT) {
			rc = cam_mem_get_cpu_buf(io_cfg->mem_handle[0],
				(uint64_t *)&buf_addr, &buf_size);
			CAM_DBG(CAM_EEPROM, "buf_addr : %pK, buf_size : %zu\n",
				(void *)buf_addr, buf_size);

			read_buffer = (uint8_t *)buf_addr;
			if (!read_buffer) {
				CAM_ERR(CAM_EEPROM,
					"invalid buffer to copy data");
				return -EINVAL;
			}
			read_buffer += io_cfg->offsets[0];

			if (buf_size < e_ctrl->cal_data.num_data) {
				CAM_ERR(CAM_EEPROM,
					"failed to copy, Invalid size");
				return -EINVAL;
			}

			CAM_DBG(CAM_EEPROM, "copy the data, len:%d",
				e_ctrl->cal_data.num_data);
			memcpy(read_buffer, e_ctrl->cal_data.mapdata,
					e_ctrl->cal_data.num_data);

		} else {
			CAM_ERR(CAM_EEPROM, "Invalid direction");
			rc = -EINVAL;
		}
	}
	return rc;
}

/**
 * cam_eeprom_pkt_parse - Parse csl packet
 * @e_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
static int32_t cam_eeprom_pkt_parse(struct cam_eeprom_ctrl_t *e_ctrl, void *arg)
{
	int32_t                         rc = 0;
	struct cam_control             *ioctl_ctrl = NULL;
	struct cam_config_dev_cmd       dev_config;
	uint64_t                        generic_pkt_addr;
	size_t                          pkt_len;
	struct cam_packet              *csl_packet = NULL;
	struct cam_eeprom_soc_private  *soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;

//HTC_START
	struct device_node *of_node = e_ctrl->soc_info.dev->of_node;
	struct cam_eeprom_soc_private     *eb_info;
	uint32_t cellindex = 5; //initialize as 5
	if (0 > of_property_read_u32(of_node, "cell-index", &cellindex)){
		CAM_ERR(CAM_EEPROM, "failed to get cell-index from dsti");
	}
//HTC_END

	ioctl_ctrl = (struct cam_control *)arg;

	if (copy_from_user(&dev_config, (void __user *) ioctl_ctrl->handle,
		sizeof(dev_config)))
		return -EFAULT;
	rc = cam_mem_get_cpu_buf(dev_config.packet_handle,
		(uint64_t *)&generic_pkt_addr, &pkt_len);
	if (rc) {
		CAM_ERR(CAM_EEPROM,
			"error in converting command Handle Error: %d", rc);
		return rc;
	}

	if (dev_config.offset > pkt_len) {
		CAM_ERR(CAM_EEPROM,
			"Offset is out of bound: off: %lld, %zu",
			dev_config.offset, pkt_len);
		return -EINVAL;
	}

	csl_packet = (struct cam_packet *)
		(generic_pkt_addr + dev_config.offset);
	switch (csl_packet->header.op_code & 0xFFFFFF) {
	case CAM_EEPROM_PACKET_OPCODE_INIT:
		if (e_ctrl->userspace_probe == false) {
			rc = cam_eeprom_parse_read_memory_map(
					e_ctrl->soc_info.dev->of_node, e_ctrl);
			if (rc < 0) {
				CAM_ERR(CAM_EEPROM, "Failed: rc : %d", rc);
				return rc;
			}
			rc = cam_eeprom_get_cal_data(e_ctrl, csl_packet);
			vfree(e_ctrl->cal_data.mapdata);
			vfree(e_ctrl->cal_data.map);
			e_ctrl->cal_data.num_data = 0;
			e_ctrl->cal_data.num_map = 0;
			CAM_DBG(CAM_EEPROM,
				"Returning the data using kernel probe");
			break;
		}
		rc = cam_eeprom_init_pkt_parser(e_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_EEPROM,
				"Failed in parsing the pkt");
			return rc;
		}

		e_ctrl->cal_data.mapdata =
			vzalloc(e_ctrl->cal_data.num_data);
		if (!e_ctrl->cal_data.mapdata) {
			rc = -ENOMEM;
			CAM_ERR(CAM_EEPROM, "failed");
			goto error;
		}

		rc = cam_eeprom_power_up(e_ctrl,
			&soc_private->power_info);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "failed rc %d", rc);
			goto memdata_free;
		}

		e_ctrl->cam_eeprom_state = CAM_EEPROM_CONFIG;
		rc = cam_eeprom_read_memory(e_ctrl, &e_ctrl->cal_data);
		if (rc) {
			CAM_ERR(CAM_EEPROM,
				"read_eeprom_memory failed");
			goto power_down;
		}

		rc = cam_eeprom_get_cal_data(e_ctrl, csl_packet);
		rc = cam_eeprom_power_down(e_ctrl);
		e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
		vfree(e_ctrl->cal_data.mapdata);
		vfree(e_ctrl->cal_data.map);
		kfree(power_info->power_setting);
		kfree(power_info->power_down_setting);
		power_info->power_setting = NULL;
		power_info->power_down_setting = NULL;
		power_info->power_setting_size = 0;
		power_info->power_down_setting_size = 0;
		e_ctrl->cal_data.num_data = 0;
		e_ctrl->cal_data.num_map = 0;
		break;
//HTC_START
	case CAM_EEPROM_PACKET_OPCODE_WRITE:
		if (e_ctrl->userspace_probe == false) {
			rc = cam_eeprom_parse_read_memory_map(
					e_ctrl->soc_info.dev->of_node, e_ctrl);
			if (rc < 0) {
				CAM_ERR(CAM_EEPROM, "Failed: rc : %d", rc);
				return rc;
			}
			rc = cam_eeprom_get_cal_data(e_ctrl, csl_packet);
			kfree(e_ctrl->cal_data.mapdata);
			kfree(e_ctrl->cal_data.map);
			e_ctrl->cal_data.num_data = 0;
			e_ctrl->cal_data.num_map = 0;
			CAM_DBG(CAM_EEPROM,
				"Returning the data using kernel probe");
			break;
		}
		rc = cam_eeprom_init_pkt_parser(e_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_EEPROM,
				"Failed in parsing the pkt");
			return rc;
		}

		e_ctrl->cal_data.mapdata =
			kzalloc(e_ctrl->cal_data.num_data, GFP_KERNEL);
		if (!e_ctrl->cal_data.mapdata) {
			rc = -ENOMEM;
			CAM_ERR(CAM_EEPROM, "failed");
			goto error;
		}

		rc = cam_eeprom_power_up(e_ctrl,
			&soc_private->power_info);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "failed rc %d", rc);
			goto memdata_free;
		}

		e_ctrl->cam_eeprom_state = CAM_EEPROM_CONFIG;
		eb_info = (struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
		cam_eeprom_write_flash(e_ctrl);
		if( cellindex == 3 ) {
			eb_info->i2c_info.slave_addr = 0x80;
			pr_info("[WEEPROM] slave-addr = 0x%X, Write protection off for s5k4h9", eb_info->i2c_info.slave_addr);
			rc = cam_eeprom_update_i2c_info(e_ctrl,
				&eb_info->i2c_info);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"failed: to update i2c info rc %d",
					rc);
				return rc;
			}
			cam_eeprom_set_protection(e_ctrl);
			eb_info->i2c_info.slave_addr = 0xA0;
			pr_info("[WEEPROM] slave-addr = 0x%X, Change to write s5k4h9 eeprom ", eb_info->i2c_info.slave_addr);
			rc = cam_eeprom_update_i2c_info(e_ctrl,
				&eb_info->i2c_info);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"failed: to update i2c info rc %d",
					rc);
				return rc;
			}
		}
		cam_eeprom_write_dual_camera(e_ctrl);
		cam_eeprom_write_dual_extrainfo_camera(e_ctrl);
		if( cellindex == 3 ) {
			eb_info->i2c_info.slave_addr = 0xF0;
			pr_info("[WEEPROM] slave-addr = 0x%X, Write protection on for s5k4h9", eb_info->i2c_info.slave_addr);
			rc = cam_eeprom_update_i2c_info(e_ctrl,
				&eb_info->i2c_info);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"failed: to update i2c info rc %d",
					rc);
				return rc;
			}
			cam_eeprom_set_protection(e_ctrl);
			eb_info->i2c_info.slave_addr = 0xA1;
			pr_info("[WEEPROM] slave-addr = 0x%X, Original slave address", eb_info->i2c_info.slave_addr);
			rc = cam_eeprom_update_i2c_info(e_ctrl,
				&eb_info->i2c_info);
			if (rc) {
				CAM_ERR(CAM_EEPROM,
					"failed: to update i2c info rc %d",
					rc);
				return rc;
			}
		}

		rc = cam_eeprom_get_cal_data(e_ctrl, csl_packet);
		rc = cam_eeprom_power_down(e_ctrl);
		e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
		kfree(e_ctrl->cal_data.mapdata);
		kfree(e_ctrl->cal_data.map);
		e_ctrl->cal_data.num_data = 0;
		e_ctrl->cal_data.num_map = 0;
		break;
//HTC_END
	default:
		break;
	}
	return rc;
power_down:
	cam_eeprom_power_down(e_ctrl);
memdata_free:
	vfree(e_ctrl->cal_data.mapdata);
error:
	kfree(power_info->power_setting);
	kfree(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	vfree(e_ctrl->cal_data.map);
	e_ctrl->cal_data.num_data = 0;
	e_ctrl->cal_data.num_map = 0;
	e_ctrl->cam_eeprom_state = CAM_EEPROM_INIT;
	return rc;
}

void cam_eeprom_shutdown(struct cam_eeprom_ctrl_t *e_ctrl)
{
	int rc;
	struct cam_eeprom_soc_private  *soc_private =
		(struct cam_eeprom_soc_private *)e_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;

	if (e_ctrl->cam_eeprom_state == CAM_EEPROM_INIT)
		return;

	if (e_ctrl->cam_eeprom_state == CAM_EEPROM_CONFIG) {
		rc = cam_eeprom_power_down(e_ctrl);
		if (rc < 0)
			CAM_ERR(CAM_EEPROM, "EEPROM Power down failed");
		e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
	}

	if (e_ctrl->cam_eeprom_state == CAM_EEPROM_ACQUIRE) {
		rc = cam_destroy_device_hdl(e_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_EEPROM, "destroying the device hdl");

		e_ctrl->bridge_intf.device_hdl = -1;
		e_ctrl->bridge_intf.link_hdl = -1;
		e_ctrl->bridge_intf.session_hdl = -1;

		kfree(power_info->power_setting);
		kfree(power_info->power_down_setting);
		power_info->power_setting = NULL;
		power_info->power_down_setting = NULL;
		power_info->power_setting_size = 0;
		power_info->power_down_setting_size = 0;
	}

	e_ctrl->cam_eeprom_state = CAM_EEPROM_INIT;
}

/**
 * cam_eeprom_driver_cmd - Handle eeprom cmds
 * @e_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
int32_t cam_eeprom_driver_cmd(struct cam_eeprom_ctrl_t *e_ctrl, void *arg)
{
	int                            rc = 0;
	struct cam_eeprom_query_cap_t  eeprom_cap = {0};
	struct cam_control            *cmd = (struct cam_control *)arg;

	if (!e_ctrl || !cmd) {
		CAM_ERR(CAM_EEPROM, "Invalid Arguments");
		return -EINVAL;
	}

	if (cmd->handle_type != CAM_HANDLE_USER_POINTER) {
		CAM_ERR(CAM_EEPROM, "Invalid handle type: %d",
			cmd->handle_type);
		return -EINVAL;
	}

	mutex_lock(&(e_ctrl->eeprom_mutex));
	switch (cmd->op_code) {
	case CAM_QUERY_CAP:
		eeprom_cap.slot_info = e_ctrl->soc_info.index;
		if (e_ctrl->userspace_probe == false)
			eeprom_cap.eeprom_kernel_probe = true;
		else
			eeprom_cap.eeprom_kernel_probe = false;

		if (copy_to_user((void __user *) cmd->handle,
			&eeprom_cap,
			sizeof(struct cam_eeprom_query_cap_t))) {
			CAM_ERR(CAM_EEPROM, "Failed Copy to User");
			return -EFAULT;
			goto release_mutex;
		}
		CAM_DBG(CAM_EEPROM, "eeprom_cap: ID: %d", eeprom_cap.slot_info);
		break;
	case CAM_ACQUIRE_DEV:
		rc = cam_eeprom_get_dev_handle(e_ctrl, arg);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "Failed to acquire dev");
			goto release_mutex;
		}
		e_ctrl->cam_eeprom_state = CAM_EEPROM_ACQUIRE;
		break;
	case CAM_RELEASE_DEV:
		if (e_ctrl->cam_eeprom_state != CAM_EEPROM_ACQUIRE) {
			rc = -EINVAL;
			CAM_WARN(CAM_EEPROM,
			"Not in right state to release : %d",
			e_ctrl->cam_eeprom_state);
			goto release_mutex;
		}

		if (e_ctrl->bridge_intf.device_hdl == -1) {
			CAM_ERR(CAM_EEPROM,
				"Invalid Handles: link hdl: %d device hdl: %d",
				e_ctrl->bridge_intf.device_hdl,
				e_ctrl->bridge_intf.link_hdl);
			rc = -EINVAL;
			goto release_mutex;
		}
		rc = cam_destroy_device_hdl(e_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_EEPROM,
				"failed in destroying the device hdl");
		e_ctrl->bridge_intf.device_hdl = -1;
		e_ctrl->bridge_intf.link_hdl = -1;
		e_ctrl->bridge_intf.session_hdl = -1;
		e_ctrl->cam_eeprom_state = CAM_EEPROM_INIT;
		break;
	case CAM_CONFIG_DEV:
		rc = cam_eeprom_pkt_parse(e_ctrl, arg);
		if (rc) {
			CAM_ERR(CAM_EEPROM, "Failed in eeprom pkt Parsing");
			goto release_mutex;
		}
		break;
	default:
		CAM_DBG(CAM_EEPROM, "invalid opcode");
		break;
	}

release_mutex:
	mutex_unlock(&(e_ctrl->eeprom_mutex));

	return rc;
}


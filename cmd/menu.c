/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <mmc.h>

/* The buffer address for save file for tftp command */
#define TFTP_SAVE_RAM_ADDR 0x88000000

/* The u-boot file max size (KB) */
#define UBOOT_FILE_MAX_SIZE 512

/* 
 * The partition start sector 
 * MLO:0-1  1KB
 * U-BOOT:2-2047  1MB-1KB
 * KERNEL:2048-34815  16MB
 * ROOTFS:34816-751615 350MB
 */
#define MLO_WRITE_MMC_SECTOR_START      0x00
#define UBOOT_WRITE_MMC_SECTOR_START	0x02
#define KERNEL_WRITE_MMC_SECTOR_START   0x800
#define ROOTFS_WRITE_MMC_SECTOR_START   0x8800

#ifdef CONFIG_GENERIC_MMC

static void param_menu_usage(void)
{
	printf("\n");
	printf("#####\tu-boot-2016.03 Main menu by Eurphan\t#####\n");
	printf("[b] Boot the system\n");
	printf("[u] Download u-boot and MLO to EMMC\n");
	printf("[k] Download Linux Kernel to EMMC\n");
	printf("[f] Download File System to EMMC\n");
	printf("[r] Reset the u-boot\n");
	printf("[q] Exit from the menu\n");
	printf("Enter your selection: ");
}


static int do_menu(void)
{
	char c;
	char device;
	char cmd_buff[256];
	char tmp[128];
	char* tmpp;
	int size;

	while(1)
	{
		printf("Select the mmc device to operation, for example 0,1... :");
		device = getc();
		printf("%c\n", device);
		if ((device == 'q') || (device == 'Q'))
		{
			printf("You give up the operation!\n");
			return CMD_RET_FAILURE;
		}
		else if ((device < '0') || (device > '9'))
		{
			printf("Please select precise mmc device!\n");
			continue;
		}
		else
		{
			break;
		}
	}
	sprintf(cmd_buff, "mmc dev %c", device); /* Select the mmc device and partition */
	if (run_command(cmd_buff, 0))
	{
		printf("Please select precise mmc device!\n");
		return CMD_RET_FAILURE;
	}
	sprintf(cmd_buff, "mmc info");
	run_command(cmd_buff, 0); /* Printf the mmc infomation */

	while(1)
	{
		param_menu_usage();
		c = getc();
		printf("%c\n", c);

		switch (c)
		{
		case 'b': /* Boot the system */
		case 'B':
			sprintf(cmd_buff, "boot");
			run_command(cmd_buff, 0);
			break;
		case 'u': /* Download the u-boot */
		case 'U':
			sprintf(cmd_buff, "mmc partconf %d 1 7 1", device); /* 1:boot1 2:boot2 7:userdata */
			run_command(cmd_buff, 0);
			sprintf(cmd_buff, "tftp %x u-boot.imx", TFTP_SAVE_RAM_ADDR); /* Get the u-boot.imx file */
			if (run_command(cmd_buff, 0))
			{
				printf("Get the u-boot file failure!\n");
				break;
			}
			tmpp = tmp;
			tmpp = getenv("filesize"); /* Get the u-boot file size */
			size = (int)simple_strtoul(tmpp, NULL, 16);
			if (size % 512)
				size = size / 512 + 1;
			else
				size = size / 512;
			sprintf(cmd_buff, "mmc erase %x %d", MLO_WRITE_MMC_SECTOR_START, size + 2); /* Erase the u-boot and MLO partition */
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device erase failure!\n");
				break;
			}
			sprintf(cmd_buff, "mmc write %x %x %d", TFTP_SAVE_RAM_ADDR, UBOOT_WRITE_MMC_SECTOR_START, size); /* Write the u-boot file to mmc device */
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device write failure!\n");
				break;
			}
			sprintf(cmd_buff, "tftp %x MLO", TFTP_SAVE_RAM_ADDR); /* Get the MLO file */
			if (run_command(cmd_buff, 0))
			{
				printf("Get the MLO file failure!\n");
				break;
			}
			sprintf(cmd_buff, "mmc write %x %x 2", TFTP_SAVE_RAM_ADDR, MLO_WRITE_MMC_SECTOR_START); /* Write the MLO file to mmc device */
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device write failure!\n");
				break;
			}
			break;
		case 'k': /* Download the kernel and device tree partition */
		case 'K':
			sprintf(cmd_buff, "tftp %x kernel.vfat", TFTP_SAVE_RAM_ADDR); /* Get kernel.vfat file */
			if (run_command(cmd_buff, 0))
			{
				printf("get kernel.vfat failure!\n");
				break;
			}
			tmpp = tmp;
			tmpp = getenv("filesize"); /* Get the kernel.vfat file size */
			size = (int)simple_strtoul(tmpp, NULL, 16);
			if (size % 512)
				size = size / 512 + 1;
			else
				size = size / 512;
			sprintf(cmd_buff, "mmc erase %x %x", KERNEL_WRITE_MMC_SECTOR_START, size);
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device erase failure!\n");
				break;
			}
			sprintf(cmd_buff, "mmc write %x %x %x", TFTP_SAVE_RAM_ADDR, KERNEL_WRITE_MMC_SECTOR_START, size);
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device write failure");
				break;
			}
			break;
		case 'f': /* Download the root file system */
		case 'F':
			sprintf(cmd_buff, "tftp %x rootfs.ext4", TFTP_SAVE_RAM_ADDR); /* Get kernel.vfat file */
			if (run_command(cmd_buff, 0))
			{
				printf("Get rootfs.ext4 failure!\n");
				break;
			}
			tmpp = tmp;
			tmpp = getenv("filesize"); /* Get the rootfs.ext4 file size */
			size = (int)simple_strtoul(tmpp, NULL, 16);
			if (size % 512)
				size = size / 512 + 1;
			else
				size = size / 512;
			sprintf(cmd_buff, "mmc erase %x %x", ROOTFS_WRITE_MMC_SECTOR_START, size);
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device erase failure!\n");
				break;
			}
			sprintf(cmd_buff, "mmc write %x %x %x", TFTP_SAVE_RAM_ADDR, ROOTFS_WRITE_MMC_SECTOR_START, size);
			if (run_command(cmd_buff, 0))
			{
				printf("MMC device write failure");
				break;
			}

			break;
		case 'r': /* Reset */
		case 'R':
			sprintf(cmd_buff, "reset");
			run_command(cmd_buff, 0);
			break;
		case 'q': /* Exit */
		case 'Q':
			return CMD_RET_SUCCESS;
			break;
		}
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	menu, 1, 1, do_menu,
	"menu - display a menu, to select the items to do something",
	"- display a menu, to select the items to do something, example upgrade the kernel dtb and file system\n"
	);

#endif /* !CONFIG_GENERIC_MMC */

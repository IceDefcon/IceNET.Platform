// SPDX-License-Identifier: GPL-2.0-only
/*
 * Ram backed block device driver.
 *
 * Copyright (C) 2007 Nick Piggin
 * Copyright (C) 2007 Novell Inc.
 *
 * Parts derived from drivers/block/rd.c, and drivers/block/loop.c, copyright
 * of their respective owners.
 */

/*!
 *
 * Re-factored: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef RAM_DISK_H
#define RAM_DISK_H

#define ICE_RAM_DISK_MAJOR   2

#define ICE_RAM_DISK_SIZE       32 /* Number of sectors per ramDisk */
#define ICE_RAM_DISK_AMOUNT     2  /* Abount of ramDisk's */
#define ICE_PAGE_SHIFT          12 /* Standard page size is 4 KB (2^12 = 4096 bytes) */
#define ICE_SECTOR_SHIFT        9  /* Standard sector size is 512 bytes (2^9 = 512 bytes) */
#define ICE_PAGE_SECTORS_SHIFT  3  /* (ICE_PAGE_SHIFT - SECTOR_SHIFT) = 12 - 9 = 3 */
#define ICE_PAGE_SECTORS        8  /* 1 << ICE_PAGE_SECTORS_SHIFT = 1 << 3 = 8 sectors per page */
#define ICE_FREE_BATCH          16 /* Adjust as needed for batch operations */

void *ramDiskGetPointer(sector_t sector);
void ramDiskReleasePointer(void *page_data);

int ramDiskInit(void);
void ramDiskDestroy(void);

#endif // RAM_DISK_H

/*
 * include/linux/nvmap.h
 *
 * structure declarations for nvmem and nvmap user-space ioctls
 *
 * Copyright (c) 2009, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/ioctl.h>
#include <linux/file.h>
#if defined(CONFIG_ICS)
#include <linux/rbtree.h>
#endif

#if !defined(__KERNEL__)
#define __user
#endif

#ifndef __NVMAP_H
#define __NVMAP_H

#define NVMAP_HEAP_SYSMEM  (1ul<<31)
#define NVMAP_HEAP_IOVMM   (1ul<<30)

/* common carveout heaps */
#define NVMAP_HEAP_CARVEOUT_IRAM    (1ul<<29)
#if defined(CONFIG_ICS)
#define NVMAP_HEAP_CARVEOUT_VPR     (1ul<<28)
#endif
#define NVMAP_HEAP_CARVEOUT_GENERIC (1ul<<0)

#define NVMAP_HEAP_CARVEOUT_MASK    (NVMAP_HEAP_IOVMM - 1)

/* allocation flags */
#define NVMAP_HANDLE_UNCACHEABLE     (0x0ul << 0)
#define NVMAP_HANDLE_WRITE_COMBINE   (0x1ul << 0)
#define NVMAP_HANDLE_INNER_CACHEABLE (0x2ul << 0)
#define NVMAP_HANDLE_CACHEABLE       (0x3ul << 0)
#define NVMAP_HANDLE_CACHE_FLAG      (0x3ul << 0)

#define NVMAP_HANDLE_SECURE          (0x1ul << 2)


#if defined(__KERNEL__)

#if !defined(CONFIG_ICS)
struct nvmap_handle_ref;
#endif
struct nvmap_handle;
struct nvmap_client;
struct nvmap_device;
#if defined(CONFIG_ICS)
#define nvmap_convert_handle_u2k(h) (h)
#endif

#define nvmap_ref_to_handle(_ref) (*(struct nvmap_handle **)(_ref))
#define nvmap_id_to_handle(_id) ((struct nvmap_handle *)(_id))

struct nvmap_pinarray_elem {
	__u32 patch_mem;
	__u32 patch_offset;
	__u32 pin_mem;
	__u32 pin_offset;
#if defined(CONFIG_ICS)
	__u32 reloc_shift;
#endif
};

#if defined(CONFIG_ICS)
/* handle_ref objects are client-local references to an nvmap_handle;
 * they are distinct objects so that handles can be unpinned and
 * unreferenced the correct number of times when a client abnormally
 * terminates */
struct nvmap_handle_ref {
        struct nvmap_handle *handle;
        struct rb_node  node;
        atomic_t        dupes;  /* number of times to free on file close */
        atomic_t        pin;    /* number of times to unpin on free */
};
#endif

struct nvmap_client *nvmap_create_client(struct nvmap_device *dev,
					 const char *name);

struct nvmap_handle_ref *nvmap_alloc(struct nvmap_client *client, size_t size,
				     size_t align, unsigned int flags);

void nvmap_free(struct nvmap_client *client, struct nvmap_handle_ref *r);

void *nvmap_mmap(struct nvmap_handle_ref *r);

void nvmap_munmap(struct nvmap_handle_ref *r, void *addr);

struct nvmap_client *nvmap_client_get_file(int fd);

struct nvmap_client *nvmap_client_get(struct nvmap_client *client);

void nvmap_client_put(struct nvmap_client *c);

#if !defined(CONFIG_ICS)
unsigned long nvmap_pin(struct nvmap_client *c, struct nvmap_handle_ref *r);

unsigned long nvmap_handle_address(struct nvmap_client *c, unsigned long id);
#else
phys_addr_t nvmap_pin(struct nvmap_client *c, struct nvmap_handle_ref *r);

phys_addr_t nvmap_handle_address(struct nvmap_client *c, unsigned long id);
#endif

void nvmap_unpin(struct nvmap_client *client, struct nvmap_handle_ref *r);

int nvmap_pin_array(struct nvmap_client *client, struct nvmap_handle *gather,
		    const struct nvmap_pinarray_elem *arr, int nr,
		    struct nvmap_handle **unique);

void nvmap_unpin_handles(struct nvmap_client *client,
			 struct nvmap_handle **h, int nr);

#if !defined(CONFIG_ICS)
int nvmap_patch_wait(struct nvmap_client *client,
		     struct nvmap_handle *patch,
		     u32 patch_offset, u32 patch_value);
#else
int nvmap_patch_word(struct nvmap_client *client,
                     struct nvmap_handle *patch,
                     u32 patch_offset, u32 patch_value);
#endif

struct nvmap_platform_carveout {
	const char *name;
	unsigned int usage_mask;
#if defined(CONFIG_ICS)
	phys_addr_t base;
#else
	unsigned long base;
#endif
	size_t size;
	size_t buddy_size;
};

struct nvmap_platform_data {
	const struct nvmap_platform_carveout *carveouts;
	unsigned int nr_carveouts;
};

extern struct nvmap_device *nvmap_dev;

#endif

#endif

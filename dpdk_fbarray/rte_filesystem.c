/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include "rte_filesystem.h"
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "rte_log.h"
static char runtime_dir[PATH_MAX];

static const char *default_runtime_dir = "/var/run";

int eal_create_runtime_dir(void)
{
	const char *directory = default_runtime_dir;
	const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
	const char *fallback = "/tmp";
	char tmp[PATH_MAX];
	int ret;

	if (getuid() != 0) 
	{
		/* try XDG path first, fall back to /tmp */
		if (xdg_runtime_dir != NULL)
			directory = xdg_runtime_dir;
		else
			directory = fallback;
	}
	/* create DPDK subdirectory under runtime dir */
	ret = snprintf(tmp, sizeof(tmp), "%s/dpdk", directory);
	if (ret < 0 || ret == sizeof(tmp)) 
	{
		RTE_LOG(ERR, EAL, "Error creating DPDK runtime path name\n");
		return -1;
	}

	/* create prefix-specific subdirectory under DPDK runtime dir */
	ret = snprintf(runtime_dir, sizeof(runtime_dir), "%s/%s",
			tmp, "HugePage_2M");
	if (ret < 0 || ret == sizeof(runtime_dir)) 
	{
		RTE_LOG(ERR, EAL, "Error creating prefix-specific runtime path name\n");
		return -1;
	}

	/* create the path if it doesn't exist. no "mkdir -p" here, so do it
	 * step by step.
	 */
	ret = mkdir(tmp, 0700);
	if (ret < 0 && errno != EEXIST) 
	{
		RTE_LOG(ERR, EAL, "Error creating '%s': %s\n",
			tmp, strerror(errno));
		return -1;
	}

	ret = mkdir(runtime_dir, 0700);
	if (ret < 0 && errno != EEXIST) 
	{
		RTE_LOG(ERR, EAL, "Error creating '%s': %s\n",
			runtime_dir, strerror(errno));
		return -1;
	}

	return 0;
}

const char * eal_get_runtime_dir(void)
{
	return runtime_dir;
}
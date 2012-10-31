/*
 * libfdt - Flat Device Tree manipulation
 *	Basic testcase for read-only access
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include <fdt.h>
#include <libfdt.h>

#include "tests.h"
#include "testdata.h"

int main(int argc, char *argv[])
{
	void *fdt, *fdt1, *fdt2;
	char *buf;
	int shuntsize1, shuntsize2;
	int bufsize, len, err, common_offset;
	const void *nodep;

	test_init(argc, argv);
	if (argc < 2)
		CONFIG("Usage: %s <dtb file 1> <dtb file to overlay> ", argv[0]);

	fdt = load_blob(argv[1]);
	fdt2 = load_blob(argv[2]);

	shuntsize1 = ALIGN(fdt_totalsize(fdt) / 2, sizeof(uint64_t));
	shuntsize2 = ALIGN(fdt_totalsize(fdt2) / 2, sizeof(uint64_t));
	bufsize = fdt_totalsize(fdt) + fdt_totalsize(fdt2) + shuntsize1 + shuntsize2;
	buf = xmalloc(bufsize);

	fdt1 = buf;
	err = fdt_open_into(fdt, fdt1, bufsize);
	if (err)
		FAIL("Failed to open tree into new buffer: %s",
		     fdt_strerror(err));

	err = fdt_overlay(buf, fdt2);
	if (err)
		FAIL("fdt_overlay failed: %s",
		     fdt_strerror(err));

	err = fdt_path_offset(buf, "/a");
	if (err < 0)
		FAIL("node /a not found in overlayed tree: %s",
		     fdt_strerror(err));

	err = fdt_path_offset(buf, "/b");
	if (err < 0)
		FAIL("node /b not found in overlayed tree: %s",
		     fdt_strerror(err));

	err = fdt_path_offset(buf, "/common");
	if (err < 0)
		FAIL("node /common not found in overlayed tree: %s",
		     fdt_strerror(err));

	common_offset = err;

	nodep = fdt_getprop(buf, common_offset, "prop-a", &len);
	if (!nodep)
		FAIL("node /common/prop-a not found in overlayed tree");
	if (len != sizeof(int))
		FAIL("node /common/prop-a has invalid size %d", len);

	nodep = fdt_getprop(buf, common_offset, "prop-b", &len);
	if (!nodep)
		FAIL("name /common/prop-b not found in overlayed tree");
	if (len != sizeof(int))
		FAIL("node /common/prop-b has invalid size %d", len);

	nodep = fdt_getprop(buf, common_offset, "both", &len);
	if (!nodep)
		FAIL("name /common/both not found in overlayed tree");
	if (len != sizeof(int))
		FAIL("node /common/both has invalid size %d", len);
	if (fdt32_to_cpu(*(const int *) nodep) != 2)
		FAIL("node /common/both is expected to have value 2, got %d",
			fdt32_to_cpu(*(const int *) nodep));

	PASS();
}

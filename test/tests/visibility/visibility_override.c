/**
* @file test/tests/visibility/visibility_override.c
*
* @brief Test: accessing a weak symbol that is overridden
*
* SPDX-FileCopyrightText: 2008-2021 HPDCS Group <rootsim@googlegroups.com>
* SPDX-License-Identifier: GPL-3.0-only
*/
#include <string.h>

char *core_version = "This is a new variable";

int main(void)
{
	return strcmp(core_version, "3.0.0-alpha.3") == 0;
}

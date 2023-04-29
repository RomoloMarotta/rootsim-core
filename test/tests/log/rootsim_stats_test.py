#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2008-2023 HPDCS Group <rootsim@googlegroups.com>
# SPDX-License-Identifier: GPL-3.0-only
import os
import re
import runpy
import sys
import textwrap


def regex_get():
    count_regex = r"(\d+)"
    float_regex = r"(\d+(?:\.\d+)?)"
    measure_regex = f"{float_regex}(?:[munKMGTPEZ]i?)?"

    stats_regex_str = f'''\
        TOTAL SIMULATION TIME ..... : {measure_regex}s
        TOTAL PROCESSING TIME ..... : {measure_regex}s
        TOTAL KERNELS ............. : {count_regex}
        TOTAL THREADS ............. : {count_regex}
        TOTAL LPS ................. : {count_regex}
        TOTAL EXECUTED EVENTS ..... : {count_regex}
        TOTAL COMMITTED EVENTS..... : {count_regex}
        TOTAL REPROCESSED EVENTS... : {count_regex}
        TOTAL SILENT EVENTS........ : {count_regex}
        TOTAL ROLLBACKS EXECUTED... : {count_regex}
        TOTAL ANTIMESSAGES......... : {count_regex}
        ROLLBACK FREQUENCY......... : {float_regex}%
        ROLLBACK LENGTH............ : {float_regex}
        EFFICIENCY................. : {float_regex}%
        AVERAGE EVENT COST......... : {measure_regex}s
        AVERAGE CHECKPOINT COST.... : {measure_regex}s
        AVERAGE RECOVERY COST...... : {measure_regex}s
        AVERAGE CHECKPOINT SIZE.... : {measure_regex}B
        LAST COMMITTED GVT ........ : {float_regex}
        NUMBER OF GVT REDUCTIONS... : {count_regex}
        SIMULATION TIME SPEED...... : {float_regex}
        AVERAGE MEMORY USAGE....... : {measure_regex}B
        PEAK MEMORY USAGE.......... : {measure_regex}B
        '''

    stats_regex_str = textwrap.dedent(stats_regex_str)
    return re.compile(stats_regex_str, re.MULTILINE)


def test_init():
    if len(sys.argv) < 3:
        raise RuntimeError("Need the rootsim_stats.py path and the test folder containing the statistics binaries!")

    script_path = sys.argv[1]
    test_folder = sys.argv[2]
    del sys.argv[2:]
    return script_path, test_folder


def test_stats_file(base_name, expected):
    full_base_name = os.path.join(bin_folder, base_name)
    if not os.path.isfile(full_base_name + ".bin"):
        sys.exit(1)
    sys.argv[1] = full_base_name + ".bin"
    runpy.run_path(path_name=rs_script_path, run_name="__main__")
    with open(full_base_name + ".txt", "r", encoding="utf8") as report_file:
        data = report_file.read()

    match = stats_regex.fullmatch(data)
    if match is None:
        sys.exit(1)

    for i, expected_field in enumerate(expected):
        if expected_field == 'NZ':
            if float(match[i + 1]) == 0:
                sys.exit(1)
        elif expected_field != match[i + 1]:
            sys.exit(1)


if __name__ == "__main__":
    rs_script_path, bin_folder = test_init()
    stats_regex = regex_get()
    test_stats_file("empty_stats", ["NZ", "0", "1", "2", "0", "0", "0", "0", "0", "0", "0", "0.00", "0.00", "100.00",
                                    "0", "0", "0", "0", "0.0", "0", "0.0", "0", "NZ"])
    test_stats_file("single_gvt_stats", ["NZ", "0", "1", "2", "16", "0", "0", "0", "0", "0", "0", "0.00", "0.00",
                                         "100.00", "0", "0", "0", "0", "0.0", "1", "0.0", "NZ", "NZ"])
    test_stats_file("multi_gvt_stats", ["NZ", "0", "1", "2", "16", "0", "0", "0", "0", "0", "0", "0.00", "0.00",
                                        "100.00", "0", "0", "0", "0", "48.56", "4", "12.14", "NZ", "NZ"])
    test_stats_file("measures_stats", ["NZ", "0", "1", "2", "16", "156", "102", "24", "30", "20", "60", "15.87", "1.20",
                                       "80.95", "0", "0", "0", "0", "0.0", "1", "0.0", "NZ", "NZ"])

    # TODO: test the actual RSStats python object

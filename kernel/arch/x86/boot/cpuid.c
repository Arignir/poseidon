/******************************************************************************\
**
**  This file is part of the Poseidon Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2020 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\******************************************************************************/

#include <poseidon/poseidon.h>
#include <arch/x86/cpuid.h>
#include <lib/string.h>
#include <lib/log.h>

struct cpuid cpu_features;

static char const * const features_name[ARRAY_LENGTH(cpu_features.features.values)][32] = {
    // EDX when CPUID.EAX=0x1
    [0] = {
        [0] = "fpu",
        [1] = "vme",
        [2] = "de",
        [3] = "pse",
        [4] = "tsc",
        [5] = "msr",
        [6] = "pae",
        [7] = "mce",
        [8] = "cx8",
        [9] = "apic",
        [10] = NULL,
        [11] = "sep",
        [12] = "mtrr",
        [13] = "pge",
        [14] = "mca",
        [15] = "cmov",
        [16] = "pat",
        [17] = "pse36",
        [18] = "psn",
        [19] = "clflush",
        [20] = NULL,
        [21] = "ds",
        [22] = "acpi",
        [23] = "mmx",
        [24] = "fxsr",
        [25] = "sse",
        [26] = "sse2",
        [27] = "ss",
        [28] = "htt",
        [29] = "tm",
        [30] = NULL,
        [31] = "bpe",
    },
    // ECX when CPUID.EAX=0x1
    [1] = {
        [0] = "sse3",
        [1] = "pclmulqdq",
        [2] = "dtes64",
        [3] = "monitor",
        [4] = "ds_cpl",
        [5] = "vmx",
        [6] = "smx",
        [7] = "eist",
        [8] = "tm2",
        [9] = "ssse3",
        [10] = "cnxt_id",
        [11] = "sdbg",
        [12] = "fma",
        [13] = "cmpxchg16b",
        [14] = "xtpr",
        [15] = "pdcm",
        [16] = NULL,
        [17] = "pcid",
        [18] = "dca",
        [19] = "sse4_1",
        [20] = "sse4_2",
        [21] = "x2apic",
        [22] = "movbe",
        [23] = "popcnt",
        [24] = "tsc_deadline",
        [25] = "aes",
        [26] = "xsave",
        [27] = "osxsave",
        [28] = "avx",
        [29] = "f16c",
        [30] = "rdrand",
        [31] = NULL,
    },
    // EBX when CPUID.EAX=0x7 and ECX=0
    [2] = {
        [0] = "fsgsbase",
        [1] = "tsc_adjust",
        [2] = "sgx",
        [3] = "bmi1",
        [4] = "hle",
        [5] = "avx2",
        [6] = "fdp_excptn_only",
        [7] = "smep",
        [8] = "bmi2",
        [9] = "erms",
        [10] = "invpcid",
        [11] = "rtm",
        [12] = "rdt_m",
        [13] = "fpu_cs_ds_depr",
        [14] = "mpx",
        [15] = "rdt_a",
        [16] = NULL,
        [17] = NULL,
        [18] = "rdseed",
        [19] = "adx",
        [20] = "smap",
        [21] = NULL,
        [22] = NULL,
        [23] = "clflushopt",
        [24] = "clwb",
        [25] = "intel_pt",
        [26] = NULL,
        [27] = NULL,
        [28] = NULL,
        [29] = "sha",
        [30] = NULL,
        [31] = NULL,
    },
    // EBX when CPUID.EAX=0x7 and ECX=0
    [3] = {
        [0] = "prefetchwt1",
        [1] = NULL,
        [2] = "umip",
        [3] = "pku",
        [4] = "ospke",
        [5] = NULL,
        [6] = NULL,
        [7] = NULL,
        [8] = NULL,
        [9] = NULL,
        [10] = NULL,
        [11] = NULL,
        [12] = NULL,
        [13] = NULL,
        [14] = NULL,
        [15] = NULL,
        [16] = NULL,
        [17] = NULL, //
        [18] = NULL, //
        [19] = NULL, // The value of MAWAU isn't a flag.
        [20] = NULL, //
        [21] = NULL, //
        [22] = "rdpid",
        [23] = NULL,
        [24] = NULL,
        [25] = NULL,
        [26] = NULL,
        [27] = NULL,
        [28] = NULL,
        [29] = NULL,
        [30] = "sgx_lc",
        [31] = NULL,
    },
};

void
load_cpuid_features(void)
{
    memset(&cpu_features, 0, sizeof(cpu_features));

    uint32 *vendor_id = (uint32 *)cpu_features.vendor_id;

    // Load the vendor string, using the CPUID instruction with EAX=0,
    // into EBX, EDX and ECX.
    asm volatile(
        "cpuid"
        :
            "=a"(cpu_features.max_cpuid),
            "=b"(*vendor_id),
            "=d"(*(vendor_id + 1)),
            "=c"(*(vendor_id + 2))
        : "a"(0x0)
        :
    );

    /*
    ** Load CPUID.EAX=0x1
    ** This returns:
    **   * Model, familiy and stepping information in EAX
    **   * Miscellaneous information in EBX
    **   * A list of some features the CPU support in ECX
    **   * A list of some features the CPU support in EDX
    */
    if (cpu_features.max_cpuid >= 0x1) {
        uint32 ebx;

        asm volatile(
            "cpuid"
            :
                "=a"(cpu_features.version.value),
                "=b"(ebx),
                "=c"(cpu_features.features.value_1_ecx),
                "=d"(cpu_features.features.value_1_edx)
            : "a"(0x1)
            :
        );

        /*
        ** Calculate the display model and family ID, according to the Intel
        ** specification.
        */
        cpu_features.display_family = cpu_features.version.family_id;
        if (cpu_features.version.family_id == 0xF) {
            cpu_features.display_family += cpu_features.version.extended_family_id;
        }

        cpu_features.display_model = cpu_features.version.model_id;
        if (cpu_features.version.family_id == 0x6 || cpu_features.version.family_id == 0xF) {
            cpu_features.display_model += (cpu_features.version.extended_model_id << 4);
        }

        /*
        ** Move the content of EBX into some fields of cpuid.
        **
        ** We did not use the common struct/union pattern here because they
        ** cannot really be unified under a common name.
        */

        cpu_features.brand_idx = ebx & 0xF;
        cpu_features.clflush_size = ((ebx >> 8) & 0xF) * 8;
        cpu_features.max_logical_cpu = (ebx >> 16) & 0xF;
        cpu_features.initial_apic_id = (ebx >> 24) & 0xF;
    }

    /*
    ** Load CPUID.EAX=0x7, ECX=0
    ** This returns:
    **   * A list of some features the CPU support in EBX
    **   * A list of some features the CPU support in ECX
    */
    if (cpu_features.max_cpuid >= 0x7) {
        asm volatile(
            "cpuid"
            :
                "=b"(cpu_features.features.value_7_0_ebx),
                "=c"(cpu_features.features.value_7_0_ecx)
            : "a"(0x7), "c"(0x0)
            : "edx"
        );

    }

    // Load the maximum input value for extended function CPUID information.
    asm volatile(
        "cpuid"
        : "=a"(cpu_features.max_extended_cpuid)
        : "a"(0x80000000)
        : "ebx", "ecx", "edx"
    );

    // Load the brand information string, using multiple CPUID calls.
    if (cpu_features.max_extended_cpuid >= 0x80000004) {
        uint32 *brand = (uint32 *)cpu_features.brand;

        for (uint i = 0; i < 3; ++i) {
            asm volatile(
                "cpuid"
                :
                    "=a"(*(brand + i * 4 + 0)),
                    "=b"(*(brand + i * 4 + 1)),
                    "=c"(*(brand + i * 4 + 2)),
                    "=d"(*(brand + i * 4 + 3))
                : "a"(0x80000002 + i)
                :
            );
        }
    }
}

/*
** Dump the findings of the `load_cpuid_features()` to the console in a
** user-readable fashion.
*/
void
dump_cpuid(void)
{
    logln("vendor_id        | %s", cpu_features.vendor_id);
    logln("family           | %i", cpu_features.display_family);
    logln("model            | %i", cpu_features.display_model);
    logln("model name       | %s", cpu_features.brand);
    logln("stepping         | %i", cpu_features.version.stepping_id);
    logln("clflush size     | %i", cpu_features.clflush_size);

    log("flag             |");
    for (uint i = 0; i < ARRAY_LENGTH(cpu_features.features.values); ++i) {
        for (uint j = 0; j < 32; ++j) {
            char const *feature_name = features_name[i][j];
            if (feature_name) { // feature_name is NULL for reserved bytes
                if ((cpu_features.features.values[i] >> j & 0x1)) {
                    log(" %s", feature_name);
                }
            }
        }
    }
    logln("");
}

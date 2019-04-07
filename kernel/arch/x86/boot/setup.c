/******************************************************************************\
**
**  This file is part of the Poseidon Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2019 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\******************************************************************************/

/*
** Continue the initialisation of the CPU.
*/

#include <poseidon/boot/init_hook.h>
#include <arch/x86/idt.h>

/*
** Continue the initialisation of the CPU.
*/
__boot_text
static
status_t
setup(void)
{
	setup_idt();
	return (OK);
}

REGISTER_INIT_HOOK(setup, &setup, INIT_LEVEL_ARCH_EARLY);

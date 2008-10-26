//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Registers.h"

// Some operations require interrupts disabled. We create a stack here -
// when the index reaches zero, we enable interrupts. when it becomes nonzero,
// we disable interrupts. That way, if two nested start/stop calls take place,
// the inner will not disrupt the outer. TODO: for SMP make this a semaphore!
static volatile uint32_t interrupts_enabled_stack = 0;

void critical_section()
{
	if (interrupts_enabled_stack == 0)
	{
		disable_interrupts();
	}
	interrupts_enabled_stack++;
}

void end_critical_section() // TODO: check for unbalanced crit/endCrit calls.
{
	interrupts_enabled_stack--;
	if (interrupts_enabled_stack == 0)
	{
        enable_interrupts();
	}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:

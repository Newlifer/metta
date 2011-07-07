//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Minimal operator new/delete implementation.
//
#include "new.h"
#include "macros.h"
#include "panic.h"

void* operator new(size_t size) throw()
{
    PANIC("Default new called!");
    return 0;
}

void operator delete(void*)
{
    PANIC("Default delete called!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :

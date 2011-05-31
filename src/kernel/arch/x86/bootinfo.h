//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "multiboot.h"
#include "memory.h"
#include "memutils.h"
#include "iterator"
#include "module_loader.h"
#include "macros.h"
#include "new.h"
#include "memory_v1_interface.h"

// Rather arbitrary location for the bootinfo page.
static void* BOOTINFO_PAGE UNUSED_ARG = (void*)0x8000;

// TODO: We need to abstract frames module from the format of bootinfo page,
// so we add a type for memory_map and make it hide the fact that it uses the bootinfo_page
// we pass the memory_map type to frames_mod.

/// use bootinfo_t::mmap_begin/end for now, but probably bootinfo_t should return memory_map_t in request for memmap?

/*class memory_map_t
{
public:
	memory_map_t();
	memory_map_t(bootinfo_t* bi); // this hides bootinfo behind mmap type

	// memory item returned by the iterator
	class entry_t
	{
	public:
		bool is_free();
		physical_address_t start();
		size_t size();
	};

	class memory_map_iterator_t : public std::iterator<std::forward_iterator_tag, memory_map_t::memory_map_entry_t>
	{
	public:
		memory_map_iterator_t();
		memory_map_t::entry_t operator *();
    	void operator ++();
    	void operator ++(int);
		inline bool operator == (const memory_map_iterator_t& other) { return ptr == other.ptr; }
    	inline bool operator != (const memory_map_iterator_t& other) { return ptr != other.ptr; }
	};

	typedef memory_map_iterator_t iterator;
	iterator begin(); // see bootinfo_t::mmap_begin()
	iterator begin() const;
	iterator end();
	iterator end() const;
	iterator rbegin();
	iterator rbegin() const;
	iterator rend();
	iterator rend() const;
};*/

/*!
 * Provides access to boot info page structures.
 *
 * Common way of accessing it is to create an instance of bootinfo_t using placement new at the location
 * of BOOTINFO_PAGE, e.g.:
 * bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
 * Then you can add items or query items.
 */
class bootinfo_t
{
    uint32_t  magic;
    char*     free;
    address_t last_available_module_address;

    static const uint32_t BI_MAGIC = 0xbeefdea1;
    multiboot_t::mmap_entry_t* find_matching_entry(address_t start, size_t size, int& n_way);

public:
    /* Iterator for going over available physical memory map entries. */
    class mmap_iterator : public std::iterator<std::forward_iterator_tag, multiboot_t::mmap_entry_t>
    {
        address_t start;
        size_t size;
        int type;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        mmap_iterator() : ptr(0), end(0) {}
        mmap_iterator(void* entry, void* end);
        multiboot_t::mmap_entry_t* operator *(); // allows in-place modification of mmap entries
        void operator ++();
        void operator ++(int);
        inline bool operator == (const mmap_iterator& other) { return ptr == other.ptr; }
        inline bool operator != (const mmap_iterator& other) { return ptr != other.ptr; }
    };

    /* Iterator for going over available virtual memory mapping entries. */
    class vmap_iterator : public std::iterator<std::forward_iterator_tag, memory_v1_mapping>
    {
//        address_t start;
//        size_t size;
//        int type;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        vmap_iterator() : ptr(0), end(0) {}
        vmap_iterator(void* entry, void* end);
        memory_v1_mapping* operator *(); // we don't need to in-place modify memory mappings, but lets keep it this way for simplicity at the moment.
        void operator ++();
        void operator ++(int);
        inline bool operator == (const vmap_iterator& other) { return ptr == other.ptr; }
        inline bool operator != (const vmap_iterator& other) { return ptr != other.ptr; }
    };

    struct module_entry
    {
        uint64_t start, end;
        const char* name;
    };

    /* Iterator for going over available modules. */
    class module_iterator : public std::iterator<std::forward_iterator_tag, module_entry>
    {
        uint64_t mod_start;
        uint64_t mod_end;
        const char* name;
        void* ptr;
        void* end;

        void set(void* entry);

    public:
        module_iterator(void* entry, void* end);
        module_entry operator *();
        void operator ++();
        inline bool operator != (const module_iterator& other) { return ptr != other.ptr; }
    };

public:
    bootinfo_t(bool create_new = false);
    inline bool is_valid() const { return magic == BI_MAGIC && size() <= PAGE_SIZE; }
    inline size_t size() const { return reinterpret_cast<const char*>(free) - reinterpret_cast<const char*>(this); }

    inline bool will_overflow(size_t add_size)
    {
        return (size() + add_size) > PAGE_SIZE; //((free & PAGE_MASK) != ((free + add_size) & PAGE_MASK));
    }

    // NB! Module loader received from this bootinfo will modify it, so do not try to use two modules loaders from
    // two different bootinfos at once! 
    // (Don't use more than one bootinfo at a time at all, they are not concurrency-safe!)
    module_loader_t get_module_loader();

    // Load module ELF file by number.
    bool get_module(uint32_t number, address_t& start, address_t& end, const char*& name);
    // Load module by name.
//     bool get_module(const char* name, module_info_t& mod);
    bool get_cmdline(const char*& cmdline);

    mmap_iterator mmap_begin();
    mmap_iterator mmap_end();
    
    vmap_iterator vmap_begin();
    vmap_iterator vmap_end();

    module_iterator module_begin();
    module_iterator module_end();

    // Append parts of multiboot header in a format suitable for bootinfo page.
    bool append_module(uint32_t number, multiboot_t::modinfo_t* mod);
    // Appends modules loaded from the multiboot modules (initrd etc)
    bool append_module(const char* name, multiboot_t::modinfo_t* mod);

    bool append_mmap(multiboot_t::mmap_entry_t* entry);
    bool append_vmap(address_t vstart, address_t pstart, size_t size);
    bool append_cmdline(const char* cmdline);

	address_t find_top_memory_address();
	address_t find_highmem_range_of_at_least(size_t bytes);
	bool use_memory(address_t start, size_t size);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :

/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    hash_container.h
 * @author  ShizZy <shizzy@6bit.net>
 * @date    2012-11-29
 * @brief   Container object for storing a hash lookup
 *
 * @section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#ifndef COMMON_HASH_MAP_H_
#define COMMON_HASH_MAP_H_

#include "common.h"
#include "log.h"
#include <hash_map>

/// Hash container generic interface - Don't use directly, use a derived class
template <class HashType, class ValueType> class HashContainer {
    /**
     * Add (or update if already exists) a value at the specified hash in the container
     * @param hash Hash to use
     * @param value Value to update at given hash in the container
     */
    void Update(HashType hash, ValueType value);

    /**
     * Remove a hash entry in the hash container
     * @param hash Hash value of entry to remove
     */
    void Remove(HashType hash);
    
    /**
     * Fetch the value at at the given hash from the hash container
     * @param hash Hash value of entry to fetch
     * @param value Value stored at hash location. Only set if the hash is in the container.
     * @return E_OK on success (hash was found), otherwise E_ERR
     */
    int Fetch(HashType hash, ValueType& value);
};

/// A hash container that's about as simple as it gets
template <class HashType, class ValueType> class HashContainer_Simple :
    public HashContainer<HashType, ValueType> {
public:
    HashContainer_Simple(int container_size=65536) {
        num_entries_ = 0;
        container_size_ = container_size;
        container_ = new HashEntry[container_size_];
        memset(container_, 0, container_size_ * sizeof(HashEntry));
    }
    ~HashContainer_Simple() {
        delete[] container_;
    }

    void Update(HashType hash, ValueType value) {
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                container_[i].value = value;
                return;
            }
        }
        container_[num_entries_].hash = hash;
        container_[num_entries_].value = value;
        num_entries_++;

        _ASSERT_MSG(TCOMMON, num_entries_ < container_size_, 
            "Exceeded maximum hash container size!");
        return;
    }

    void Remove(HashType hash) {
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                memset(container_[i], 0, sizeof(HashEntry));
                return;
            }
        }
    }

    int Fetch(HashType hash, ValueType& value) {
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                value = container_[i].value;
                return E_OK;
            }
        }
        return E_ERR;
    }

private:
    struct HashEntry {
        HashType hash;
        ValueType value;
    };

    HashEntry* container_;

    int num_entries_;
    int container_size_;
};

/// Hash container implemented using STL hash_map
template <class HashType, class ValueType> class HashContainer_STLHashMap : 
    public HashContainer<HashType, ValueType> {
public:
    HashContainer_STLHashMap() {
    }
    ~HashContainer_STLHashMap() {
    }

    void Update(HashType hash, ValueType value) {
        hash_map_[hash] = value;
    }

    void Remove(HashType hash) {
        hash_map_.erase(hash);
    }

    int Fetch(HashType hash, ValueType& value) {
        std::hash_map<HashType, ValueType>::const_iterator res = hash_map_.find(hash);
        if (res->first != hash) {
            return E_ERR;
        }
        value = hash_map_.find(hash)->second;
        return E_OK;
    }

private:
    std::hash_map<HashType, ValueType> hash_map_;

    DISALLOW_COPY_AND_ASSIGN(HashContainer_STLHashMap);
};

#endif // COMMON_HASH_MAP_H_

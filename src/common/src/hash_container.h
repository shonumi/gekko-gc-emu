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

#ifndef COMMON_HASH_CONTAINER_H_
#define COMMON_HASH_CONTAINER_H_

#include <map>
#include "common.h"

/// Hash container generic interface - Don't use directly, use a derived class
template <class HashType, class ValueType> class HashContainer {
    /**
     * Add (or update if already exists) a value at the specified hash in the container
     * @param hash Hash to use
     * @param value Value to update at given hash in the container
     */
    ValueType* Update(HashType hash, ValueType value);

    /**
     * Remove a hash entry in the hash container
     * @param hash Hash value of entry to remove
     */
    void Remove(HashType hash);
    
    /**
     * Fetch the value at at the given hash from the hash container
     * @param hash Hash value of entry to fetch
     * @return Pointer to value stored at hash location on success (index was found), otherwise NULL
     */
    ValueType* FetchFromHash(HashType hash);

    /**
     * Fetch the value at at the given integer index from the hash container
     * @param hash Hash value of entry to fetch
     * @return Pointer to value stored at hash location on success (index was found), otherwise NULL
     */
    ValueType* FetchFromIndex(int index);

    /**
     * Get the size of the hash container
     * @return Number of elements in the hash container
     */
    int Size();
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

    ValueType* Update(HashType hash, ValueType value) {
        int i = 0;
        for (; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                container_[i].value = value;
                return &container_[i].value;
            }
        }
        container_[i].hash = hash;
        container_[i].value = value;
        num_entries_++;

        _ASSERT_MSG(TCOMMON, num_entries_ < container_size_, 
            "Exceeded maximum hash container size!");
        return &container_[i].value;
    }

    void Remove(HashType hash) {
        /* TODO(ShizZy): FixMe
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                memset(&container_[i], 0, sizeof(HashEntry));
                num_entries_--;
                return;
            }
        }*/
    }

    ValueType* FetchFromHash(HashType hash) {
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash == hash) {
                return &container_[i].value;
            }
        }
        return NULL;
    }

    ValueType* FetchFromIndex(int index) {
        int index_count = 0;
        for (int i = 0; i < num_entries_; i++) {
            if (container_[i].hash != 0) {
                if (index_count == index) {
                    return &container_[i].value;
                }
                index_count++;
            }
        }
 	    return NULL;
    }

    int Size() {
        return num_entries_;
    }

    struct HashEntry {
        HashType hash;
        ValueType value;
    };
    
private:
    HashEntry* container_;
    int num_entries_;
    int container_size_;
};

/// Hash container implemented using STL map
template <class HashType, class ValueType> class HashContainer_STLMap : 
    public HashContainer<HashType, ValueType> {

public:
    HashContainer_STLMap() {
    }
    ~HashContainer_STLMap() {
    }

    ValueType* Update(HashType hash, ValueType value) {
        map_[hash] = value;
        return &map_[hash];
    }

    void Remove(HashType hash) {
        map_.erase(hash);
    }

    ValueType* FetchFromHash(HashType hash) {
        auto itr = map_.find(hash);
        if (itr == map_.end()) {
            return NULL;
        }
        return &itr->second;
    }

    ValueType* FetchFromIndex(int index) {
	    auto itr = map_.begin();
        int i = 0;
 	    for (; i < index; ++i) {
 	        ++itr;
        }
        if (i < index) {
            return NULL;
        }
        return &itr->second;
    }

    int Size() {
        return static_cast<int>(map_.size());
    }

private:
    std::map<HashType, ValueType> map_;

    DISALLOW_COPY_AND_ASSIGN(HashContainer_STLMap);
};

#endif // COMMON_HASH_CONTAINER_H_

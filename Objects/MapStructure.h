/* Copyright (c) 2023 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef CECIL_INCL_MAPSTRUCTURE_H
#define CECIL_INCL_MAPSTRUCTURE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <STLIncludesBegin.h>
#include <list>
#include <stdexcept>
#include <STLIncludesEnd.h>

namespace se1 {

// Slow but stable and limited reimplementation of 'std::map'
// The internal mechanism of 'std::map' works with defects in C++98
template<class _T1, class _T2>
class map : public std::list<std::pair<_T1, _T2> > {
  public:
    typedef std::pair<_T1, _T2> value_type;
    typedef std::list<value_type> _Myt;
    typedef std::pair<iterator, bool> _Pairib;

  public:
    // Insert a new pair at the end or find an existing one
    _Pairib insert(const value_type &pair) {
      iterator it = find(pair.first);
      if (it != end()) return std::make_pair(it, false);

      return std::make_pair(_Myt::insert(end(), pair), true);
    };

    // Get iterator to a desired key
    iterator find(const _T1 &key) {
      for (iterator it = begin(); it != end(); it++) {
        if (it->first == key) return it;
      }

      return end();
    };

    // Get constant iterator to a desired key
    const_iterator find(const _T1 &key) const {
      for (const_iterator it = begin(); it != end(); it++) {
        if (it->first == key) return it;
      }

      return end();
    };

    // Get pair under some key
    value_type &at(const _T1 &key) {
      iterator it = find(key);
      if (it != end()) return it->second;

      throw std::range_error("se1::map::at() - key doesn't exist");
    };

    // Get constant pair under some key
    const value_type &at(const _T1 &key) const {
      const_iterator it = find(key);
      if (it != end()) return it->second;

      throw std::range_error("se1::map::at() - key doesn't exist");
    };

    // Get pair under some key or create it, if there's none
    value_type &operator[](const _T1 &key) {
      iterator it = find(key);
      if (it == end()) it = insert(end(), value_type(key, _T2()));

      return it->second;
    };

    // Get constant pair under some key
    inline const value_type &operator[](const _T1 &key) const {
      return at(key);
    };
};

}; // namespace

#endif
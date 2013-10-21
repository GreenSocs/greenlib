// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//
//   Marcus Bartholomeu
//     GreenSocs Ltd
//
//
//   This program is free software.
//
//   If you have no applicable agreement with GreenSocs Ltd, this software
//   is licensed to you, and you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   If you have a applicable agreement with GreenSocs Ltd, the terms of that
//   agreement prevail.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
//   02110-1301  USA
//
// ENDLICENSETEXT

#ifndef GS_TREE_MAP_H
#define GS_TREE_MAP_H

#include <map>
#include <string>
#include <iostream>


namespace gs {

  template <typename K, typename V>
  struct tree_map
    : public std::map<K, tree_map<K,V> >
  {
    typedef K key_type;
    typedef V value_type;
    typedef std::map<K, tree_map<K,V> > children_map;

    // Constructor:  tree_map is either a leaf or a branch;
    // always starts life as a branch
    // using the assignment operator for value_type turns it into a leaf
    // can turn back into branch using default assignment operator or
    //   operator[]
    tree_map() : m_is_leaf(false) {}

    // This node value
    value_type value;
    bool m_is_leaf;

    // Cast to value_type
    operator value_type () const
    {
      if(has_value()) return value;
      return value_type();
    }

    // TODO: use boost::enable_if to define this only if value_type is lex_t (or != 0 otherwise?)
    bool has_value() const throw()
    {
      return m_is_leaf;
    }

    // TODO: use boost::enable_if to define this only if value_type is lex_t
    template <typename ToType>
    operator ToType() const throw()
    {
      value_type tmp = (has_value() ? value : value_type());
      return tmp.template cast_to<ToType>();
    }

    template <typename KEY>
    tree_map& operator[] (const KEY& k)
    {
      return operator[](key_type(k));
    }

    tree_map& operator[] (const key_type& k)
    {
      m_is_leaf = false;
      typename children_map::iterator item = children_map::find(k);
      if (item != children_map::end())
        return item->second;  // same as children[k]
      else {
        // split at first dot
        std::string str = static_cast<const char*>(k);
        size_t dot_pos = str.find(".");
        if ( dot_pos != std::string::npos) {
          return children_map::operator[](str.substr(0,dot_pos))[str.substr(dot_pos+1)];
        }
        return children_map::operator[](k);
      }
    }

    template <typename KEY>
    const tree_map& operator[] (const KEY& k) const
    {
      return operator[](key_type(k));
    }

    const tree_map& operator[] (const key_type& k) const
    {
      typename children_map::const_iterator item = children_map::find(k);
      if (item != children_map::end())
        return item->second;  // same as children[k]
      else {
        // split at first dot
        std::string str = static_cast<const char*>(k);
        size_t dot_pos = str.find(".");
        if ( dot_pos != std::string::npos) {
          // try to find the param that is before the dot (or die)
          typename children_map::const_iterator item_child =
            children_map::find(static_cast<key_type>(str.substr(0,dot_pos)));
          if (item_child != children_map::end())
            return item_child->second[str.substr(dot_pos+1)];
          else {
            throw std::runtime_error("FATAL: key `" + str.substr(0,dot_pos) + "' not found in message.");
          }
        }
        throw std::runtime_error("FATAL: key `" + str + "' not found in message.");
      }
    }

    tree_map *find(const key_type &k)
    {
      if(has_value()) return 0;
      std::string str = static_cast<const char*>(k);
      size_t dot_pos = str.find(".");
      typename children_map::iterator item = children_map::find(
        static_cast<key_type>(str.substr(0,dot_pos)));
      if(item == children_map::end()) return 0;
      if(dot_pos == std::string::npos) return &(item->second);
      return item->second.find(static_cast<key_type>(str.substr(dot_pos+1)));
    }

    bool find(const key_type &k) const
    {
      if(has_value()) return false;
      std::string str = static_cast<const char*>(k);
      size_t dot_pos = str.find(".");
      typename children_map::const_iterator item = children_map::find(
        static_cast<key_type>(str.substr(0,dot_pos)));
      if(item == children_map::end()) return false;
      if(dot_pos == std::string::npos) return true;
      return item->second.find(static_cast<key_type>(str.substr(dot_pos+1)));
    }

    void erase(const key_type &k)
    {
      if(has_value()) return;
      std::string str = static_cast<const char*>(k);
      size_t dot_pos = str.find(".");
      typename children_map::iterator item = children_map::find(
        static_cast<key_type>(str.substr(0,dot_pos)));
      if(item == children_map::end()) return;
      if(dot_pos == std::string::npos) children_map::erase(item);
      else item->second.erase(static_cast<key_type>(str.substr(dot_pos+1)));
    }

    template <typename VALUE>
    const value_type& operator= (const VALUE& v)
    {
      return operator=(value_type(v));
    }

    const value_type& operator= (const value_type& v)
    {
      children_map::clear();
      m_is_leaf = true;
      return value = v;
    }

    size_t max_numerical_key() const
    {
      size_t max = 0;
      typename children_map::const_iterator it = children_map::begin();
      for (; it != children_map::end(); ++it) {
        if (max < static_cast<size_t>((*it).first))
          max = (*it).first;
      }
      return max;
    }

    unsigned length_as_array() const
    {
      int max_found = -1;
      int nr_found = 0;
      typename children_map::const_iterator it = children_map::begin();
      for (; it != children_map::end(); ++it) {
        int tmp = static_cast<int>((*it).first);
        // non-castable keys return "0".  only let real "0" through, and
        // return 0 if any non-integer keys exist
        // lex_t does not implement "!=", only "=="
        if((tmp <= 0) && !(key_type(0) == (*it).first)) return 0;
        if (max_found < tmp) max_found = unsigned(tmp);
        nr_found++;
      }
      // check that all indeces are present (sparse arrays not supported)
      if(nr_found == 1 + max_found) return unsigned(nr_found);
      return 0;
    }
  };


  template <typename K, typename V>
  std::ostream& operator<< (std::ostream& os, tree_map<K,V> t)
  {
    if(t.has_value()) return os << t.value;
    return os << "<>";
  }

} // namespace gs


#endif // GS_TREE_MAP_H

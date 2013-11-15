// LICENSETEXT
//
//   Copyright (C) 2009 : GreenSocs Ltd
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

#ifndef __GS_SPARSE_MEM__
#define __GS_SPARSE_MEM__

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstring>

#define GS_SPARSE_MEM_DEBUG(cmd)
//#define GS_SPARSE_MEM_DEBUG(cmd) cmd


namespace gs {
namespace mem {


template <size_t PAGE_SIZE = 1024,
          typename VALUE_TYPE = unsigned char,
          typename ADDRESS_TYPE = size_t>
class sparse_mem
{

public:

  typedef VALUE_TYPE value_type;
  typedef ADDRESS_TYPE address_type;
  typedef value_type* pointer;
  static const size_t page_size = PAGE_SIZE;

  ~sparse_mem();

  value_type& operator[](address_type addr);
  bool has(address_type addr);

  value_type* get_block(address_type start_, address_type end_);

protected:

  std::vector<address_type> ins;
  std::vector<address_type> outs;
  std::vector<value_type*> buffers;

  int lower_bound(address_type addr);
  bool insert_page(int i, address_type addr, address_type end = 0);
  void join_pages(int i, address_type start, address_type end);

};



template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
VALUE_TYPE*
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
get_block(address_type start_, address_type end_)
{
  assert( start_ < end_ );

  // relocate start and end addresses to page boundaries
  address_type start = start_/page_size * page_size;
  address_type end = (end_/page_size * page_size) + page_size;

  // find the index where the end should be
  int end_i = std::lower_bound(outs.begin(), outs.end(), end) - outs.begin();
  // if in the middle of an interval
  if (end_i != -1 and end_i != outs.size() and
      ins[end_i] < end and end < outs[end_i]) {
    // ajust to the end of that interval
    end = outs[end_i];
  }

  // find the index where the start should be
  int start_i = lower_bound(start);

  // Print debug information (when enabled)
  GS_SPARSE_MEM_DEBUG(std::cout << "get_block: "
                      << "start=" << start
                      << " start_i=" << start_i
                      << " end=" << end
                      << " end_i=" << end_i
                      << std::endl);

  // case 1: start before the first interval
  if (start_i == -1) {
    GS_SPARSE_MEM_DEBUG(std::cout << "case 1" << std::endl);
    // insert a page possibly joining with other pages
    start_i = 0;
    join_pages(start_i, start, end);
  }

  // case 2: start and end in the same (existing) interval
  else if (start_i == end_i) {
    GS_SPARSE_MEM_DEBUG(std::cout << "case 2" << std::endl);
    // nothing need to be done
  }

  // case 3: the start is withing the interval
  else if (start < outs[start_i]) {
    GS_SPARSE_MEM_DEBUG(std::cout << "case 3" << std::endl);
    // ajust to the begining of that interval
    start = ins[start_i];
    // insert a page in the current index
    join_pages(start_i, start, end);
  }

  // case 4: the start is out from the interval
  else {
    GS_SPARSE_MEM_DEBUG(std::cout << "case 4" << std::endl);
    // insert a page in the next index
    join_pages(++start_i, start, end);
  }

  // return pointer to the begining of the requested buffer
  return &buffers[start_i][start_-ins[start_i]];
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
int
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
lower_bound(address_type addr)
{
  return std::lower_bound(ins.begin(), ins.end(),
                          addr, std::less_equal<int>()) - ins.begin() - 1;
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
bool
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
insert_page(int i, address_type addr, address_type end)
{
  address_type start = addr/page_size * page_size;

  // default size is page_size
  if (end==0) end = start + page_size;

  // size must be multiple of page_size
  assert((end-start)%page_size == 0);

  // insert in the requested position, without check
  ins.insert(ins.begin()+i, start);
  outs.insert(outs.begin()+i, end);
  buffers.insert(buffers.begin()+i, new value_type[end-start]);

  GS_SPARSE_MEM_DEBUG(std::cout << "inset_page: at " << i
                      << ", from " << ins[i]
                      << " to " << outs[i]  << std::endl);
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
void
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
join_pages(int i, address_type start, address_type end)
{
  // first insert a page
  insert_page(i, start, end);

  // merge other existing pages
  int index;
  for (index=i+1; index < ins.size(); index++) {
    // merge if that block is contained withing this block
    if (outs[index] <= outs[i]) {
      memcpy(buffers[i]+(ins[index]-ins[i]),
             buffers[index],
             (outs[index]-ins[index]) * sizeof(value_type));
      // free copied buffer
      GS_SPARSE_MEM_DEBUG(std::cout << "delete_page: at " << index
                          << ", from " << ins[index]
                          << " to " << outs[index]  << std::endl);
      delete[] buffers[index];
    }
    else break;
  }

  // remove copied pages
  ins.erase(ins.begin()+i+1, ins.begin()+index);
  outs.erase(outs.begin()+i+1, outs.begin()+index);
  buffers.erase(buffers.begin()+i+1, buffers.begin()+index);
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
VALUE_TYPE&
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
operator[](address_type addr)
{
  // find index of the interval
  int i = lower_bound(addr);
  GS_SPARSE_MEM_DEBUG(std::cout << "operator[]: " << addr
                      << ", i = " << i << std::endl);

  // if smaller then the first interval (or empty)
  if (i == -1) {
    // insert page at the front
    i = 0;
    insert_page(0, addr);
  }

  // if beyond the end of this interval
  else if (addr >= outs[i]) {
    // insert a page in the next index
    ++i;
    insert_page(i, addr);
  }

  return buffers[i][addr-ins[i]];
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
bool
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
has(address_type addr)
{
  // find index of the interval
  int i = lower_bound(addr);
  GS_SPARSE_MEM_DEBUG(std::cout << "has: " << addr
                      << ", i = " << i << std::endl);

  // if smaller then the first interval (or empty), return not found
  if (i == -1) {
    return false;
  }

  // check if withing the size of this interval
  if (addr < outs[i]) {
    GS_SPARSE_MEM_DEBUG(std::cout << "withing range in interval "
                        << i << std::endl);
    GS_SPARSE_MEM_DEBUG(std::cout << "which is from " << ins[i]
                        << " to " << outs[i]  << std::endl);
    return true;
  }
  else {
    GS_SPARSE_MEM_DEBUG(std::cout << "OUT OF RANGE!" << std::endl);
    return false;
  }
}


template <size_t PAGE_SIZE,
          typename VALUE_TYPE,
          typename ADDRESS_TYPE>
sparse_mem<PAGE_SIZE,VALUE_TYPE,ADDRESS_TYPE>::
~sparse_mem()
{
  GS_SPARSE_MEM_DEBUG(std::cout << "in destructor "
                      << buffers.size() << std::endl);
  // delete allocated buffers
  for (int i=0; i < buffers.size(); i++) {
    GS_SPARSE_MEM_DEBUG(std::cout << "delete_page: at " << i
                        << ", from " << ins[i]
                        << " to " << outs[i]  << std::endl);
    delete[] buffers[i];
  }
}


} // namespace mem
} // namespace gs

#endif //ndef __GS_SPARSE_MEM__

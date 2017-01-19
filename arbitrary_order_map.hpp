/*
Copyright (c) 2012-2016 James Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

The official repository for this library is at https://github.com/odhinnsrunes/json

*/
#ifndef _ARBITRARY_ORDER_MAP_
#define _ARBITRARY_ORDER_MAP_

#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
template<class keyType, class valueType, class A = std::allocator<valueType> >
class arbitrary_order_map
{
public:
	typedef std::pair<keyType, valueType> pairType;
	typedef std::unique_ptr<pairType> ptrType;
	typedef typename std::unordered_map<keyType, pairType*>::iterator dataIterator;
	typedef typename std::vector<ptrType>::iterator keyIterator;
	typedef std::pair<keyType, pairType*> dataType;
	typedef std::pair<keyType, pairType> data2Type;
	
	arbitrary_order_map() {}

	arbitrary_order_map(const std::unordered_map<keyType, valueType> &map)
	{
		reserve(map.size());
		auto e = map.end();
		for (auto it = map.begin(); it != e; ++it) {
		   *this[it->first] = it->second;
		}
	}
	
	arbitrary_order_map(const std::map<keyType, valueType> &map)
	{
		reserve(map.size());
		auto e = map.end();
		for (auto it = map.begin(); it != e; ++it) {
			*this[it->first] = it->second;
		}
	}
	
	~arbitrary_order_map()
	{
	}

	arbitrary_order_map(const arbitrary_order_map & V)
	{
		reserve(V.size());
		auto e = V.keys.end();
		for (auto it = V.keys.begin(); it != e; ++it) {
			(*this)[(*it)->first] = ((*it)->second);
		}
	}

	arbitrary_order_map(arbitrary_order_map&& V)
	{
		keys = std::move(V.keys);
		data = std::move(V.data);
	}

	arbitrary_order_map& operator=(const arbitrary_order_map& V)
	{
		if(this == &V) {
			return *this;
		}
		this->clear();
		reserve(V.size());
		auto e = V.keys.end();
		for (auto it = V.keys.begin(); it != e; ++it) {
			(*this)[(*it)->first] = ((*it)->second);
		}

		return *this;
	}

	arbitrary_order_map& operator=(arbitrary_order_map&& V)
	{
		if(this == &V) {
			return *this;
		}
		
		keys = std::move(V.keys);
		data = std::move(V.data);

		return *this;
	}

	void reserve(size_t n)
	{
		data.reserve(n);
		keys.reserve(n);
	}
	
	valueType &operator[](const keyType &key)
	{
		dataIterator it = data.find(key);
		if (it == data.end()) {
			pairType* p = new pairType(key, valueType());
			keys.emplace_back(ptrType(p));
			data.emplace_hint(it, dataType(key, p));
			return p->second;
		}
		return ((*it).second)->second;
	}

	valueType &operator[](size_t index)
	{
		return *keys[index];
	}

	size_t size() const
	{
		return data.size();
	}

	void clear()
	{
		data.clear();
		keys.clear();
	}

	size_t count(keyType)
	{
		return 1;
	}

	bool empty() const
	{
		return data.empty();
	}

	size_t erase(keyType key)
	{
		dataIterator it = data.find(key);
		if (it == data.end()) {
			return 0;
		}
		pairType * p = (*it).second.get();
		auto e = keys.end();
		for (keyIterator it2 = keys.begin(); it2 != e; ++it2) {
			if ((*it2).get() == p) {
				keys.erase(it2);
				break;
			}
		}

		return data.erase(key);
	}

	class iterator : public std::iterator<std::input_iterator_tag, data2Type>
	{
	public:
		friend class reverse_iterator;
		friend class const_iterator;
		iterator()
		{

		}

		iterator(const keyIterator & in) : it(in)
		{

		}

		iterator(const iterator& in) : it(in.it)
		{

		}

		~iterator()
		{

		}

		iterator& operator++() 
		{
			++it;
			return *this;
		}

		iterator& operator++(int)
		{
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		iterator& operator--() 
		{
			--it;
			return *this;
		}

		iterator& operator--(int)
		{
			iterator tmp(*this);
			operator--();
			return tmp;
		}

		bool operator==(const iterator & rhs)
		{
			return it == rhs.it;
		}

		bool operator!=(const iterator & rhs)
		{
			return it != rhs.it;
		}


		pairType &operator*()
		{
			return **it;
		}

		pairType *operator->()
		{
			return (*it).get();
		}

		keyIterator &real() 
		{
			return it;
		}
		
	public:
		keyIterator	it;
	};

	typedef iterator fIterator; // Kludge for Microsoft's compiler casting
								// const iterator & it to it's parent class
								// instead of iterator in
								// reverse_iterator(const fIterator& in)...

	class const_iterator : public std::iterator<std::input_iterator_tag, data2Type>
	{
	public:
		friend class reverse_iterator;
		const_iterator()
		{

		}

		const_iterator(const typename std::vector<ptrType>::const_iterator & in) : it(in)
		{

		}

		const_iterator(keyIterator & in) : it(in)
		{

		}

		const_iterator(const const_iterator& in) : it(in.it)
		{

		}

		const_iterator(const fIterator& in) : it(in.it)
		{

		}

		~const_iterator()
		{

		}

		const_iterator& operator++() 
		{
			++it;
			return *this;
		}

		const_iterator& operator++(int)
		{
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		const_iterator& operator--() 
		{
			--it;
			return *this;
		}

		const_iterator& operator--(int)
		{
			iterator tmp(*this);
			operator--();
			return tmp;
		}

		bool operator==(const const_iterator & rhs)
		{
			return it == rhs.it;
		}

		bool operator!=(const const_iterator & rhs)
		{
			return it != rhs.it;
		}


		const pairType &operator*()
		{
			return **it;
		}

		const pairType *operator->()
		{
			return (*it).get();
		}

		const typename std::vector<ptrType>::const_iterator &real() 
		{
			return it;
		}
		
	public:
		typename std::vector<ptrType>::const_iterator 	it;
	};

	class reverse_iterator : public std::reverse_iterator<iterator>
	{
	public:
		reverse_iterator()
		{

		}

		reverse_iterator(const typename std::vector<ptrType>::reverse_iterator & in) : it(in)
		{

		}

		reverse_iterator(const reverse_iterator& in) : it(in.it)
		{

		}

		reverse_iterator(const fIterator& in) : it(typename std::vector<ptrType>::reverse_iterator(in.it))
		{

		}

		~reverse_iterator()
		{

		}

		reverse_iterator& operator++() 
		{
			++it;
			return *this;
		}

		reverse_iterator& operator++(int)
		{
			reverse_iterator tmp(*this);
			operator++();
			return tmp;
		}
		reverse_iterator& operator--() 
		{
			--it;
			return *this;
		}

		reverse_iterator& operator--(int)
		{
			reverse_iterator tmp(*this);
			operator--();
			return tmp;
		}

		bool operator==(const reverse_iterator & rhs) const
		{
			return it == rhs.it;
		}

		bool operator!=(const reverse_iterator & rhs) const
		{
			return it != rhs.it;
		}


		pairType &operator*() const
		{
			return (*it).get();
		}

		pairType *operator->() const
		{
			return (*it).get();
		}

		typename std::vector<ptrType>::reverse_iterator &real() 
		{
			return it;
		}

	public:
		typename std::vector<ptrType>::reverse_iterator 	it;
	};

	iterator begin() 
	{
		return keys.begin();
	}

	iterator end() 
	{
		return keys.end();
	}

	const_iterator begin() const
	{
		return keys.begin();
	}

	const_iterator end() const
	{
		return keys.end();
	}

	reverse_iterator rbegin() 
	{
		return keys.rbegin();
	}

	reverse_iterator rend() 
	{
		return keys.rend();
	}

	iterator find(keyType key)
	{
		dataIterator it = data.find(key);
		if (it == data.end()) {
			return keys.end();
		}
		pairType * p = (*it).second;
		auto e = keys.end();
		for (keyIterator keyIt = keys.begin(); keyIt != e; ++keyIt) {
			if ((*keyIt).get() == p) {
				return keyIt;
			}
		}
		return e;
	}

	iterator erase(iterator it)
	{
		data.erase((*it).first);
		return keys.erase(it.real());
	}

	iterator erase(iterator &start, iterator &finnish)
	{
		auto e = finnish.real();
		for (keyIterator keyIt = start.real(); keyIt != e;) {
			data.erase((*keyIt++)->first);
		}
		
		return keys.erase(start.real(), finnish.real());
	}

	bool operator==(const arbitrary_order_map &rhs) const
	{
		if (keys.size() != rhs.keys.size()) {
			return false;
		}
		auto leftIt = data.begin();
		auto rightIt = rhs.data.begin();
		for (; leftIt != data.end(); ++leftIt, ++rightIt) {
			if ((*leftIt).first != (*rightIt).first) {
				return false;
			}
			if (!((*leftIt).second->second == (*rightIt).second->second)) {
				return false;
			}
		}
		return true;
	}

	bool operator>(const arbitrary_order_map& rhs) const
	{
		auto leftIt = data.begin();
		auto rightIt = rhs.data.begin();
		for (; leftIt != data.end(); ++leftIt, ++rightIt) {
			if ((*leftIt).first > (*rightIt).first) {
				return true;
			}
			if ((*rightIt).first > (*leftIt).first) {
				return true;
			}
		}
		return (leftIt != data.end()) && (rightIt == rhs.data.end());
	}

	bool operator<(const arbitrary_order_map& rhs) const
	{
		auto leftIt = data.begin();
		auto rightIt = rhs.data.begin();
		for (; leftIt != data.end(); ++leftIt, ++rightIt) {
			if ((*leftIt).first < (*rightIt).first) {
				return true;
			}
			if ((*rightIt).first < (*leftIt).first) {
				return true;
			}
		}
		return (leftIt == data.end()) && (rightIt != rhs.data.end());
	}


	bool operator!=(const arbitrary_order_map& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator<=(const arbitrary_order_map& rhs) const
	{
		auto leftIt = data.begin();
		auto rightIt = rhs.data.begin();
		for (; leftIt != data.end(); ++leftIt, ++rightIt) {
			if ((*leftIt).first < (*rightIt).first) {
				return true;
			}
			if ((*rightIt).first < (*leftIt).first) {
				return true;
			}
		}
		return (leftIt == data.end()) && (rightIt == rhs.data.end());
	}

	bool operator>=(const arbitrary_order_map& rhs) const
	{
		auto leftIt = data.begin();
		auto rightIt = rhs.data.begin();
		for (; leftIt != data.end(); ++leftIt, ++rightIt) {
			if ((*leftIt).first > (*rightIt).first) {
				return true;
			}
			if ((*rightIt).first > (*leftIt).first) {
				return true;
			}
		}
		return (leftIt == data.end()) && (rightIt == rhs.data.end());
	}

	iterator insert(iterator at, pairType  val)
	{
		auto dit = find(val.first);
		if (dit != end()) {
			return dit;
		} else {
			pairType* n = (new pairType(val));
			iterator it = keys.emplace(at.real(), ptrType(n));
			data.emplace_hint(data.end(), dataType(val.first, n));
			return it;
		}
	}

	iterator insert(iterator start, iterator finnish)
	{
		iterator insIt = end();
		reserve(std::distance(start, finnish));
		auto e = finnish.real();
		for (keyIterator keyIt = start.real(); keyIt != e;) {
			insIt = insert(end(), *(*keyIt++));
		}
		return insIt;
	}

	iterator insert(iterator at, iterator start, iterator finnish)
	{
		iterator insIt = at;
		reserve(std::distance(start, finnish));
		auto e = finnish.real();
		for (keyIterator keyIt = start.real(); keyIt != e;) {
			insIt = insert(insIt, *(*keyIt++));
			++insIt;
		}
		return insIt;
	}
protected:
	std::unordered_map<keyType, pairType*>		data;
	std::vector<ptrType> 			keys;
};

#endif

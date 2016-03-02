#ifndef _ARBITRARY_ORDER_MAP_
#define _ARBITRARY_ORDER_MAP_

#include <map>
#include <vector>
#include <memory>

template<class keyType, class valueType, class A = std::allocator<valueType> >
class arbitrary_order_map
{
public:
//    class pairType
//    {
//    public:
//        pairType(const pairType & in) : first(in.first), second(in.second) {}
//        pairType(const keyType & in) : first(in) {}
//        pairType(const keyType & in, const valueType & inv) : first(in), second(inv) {}
//        pairType(const std::pair<keyType, valueType> & pair) : first(pair.first), second(pair.second) {}
//        keyType first;
//        valueType second;
//    };
    typedef std::pair<keyType, valueType> pairType;
    typedef std::shared_ptr<pairType> ptrType;
	typedef typename std::map<keyType, pairType*>::iterator dataIterator;
	typedef typename std::vector<ptrType>::iterator keyIterator;
    typedef std::pair<keyType, pairType*> dataType;
    typedef std::pair<keyType, pairType> data2Type;
	
    arbitrary_order_map(){}

    arbitrary_order_map(const std::map<keyType, valueType> &map)
    {
    	for (auto it = map.begin(); it != map.end(); ++it) {
    		*this[it->first] = it->second;
    	}
    }

	~arbitrary_order_map()
    {
    }

	valueType &operator[](const keyType &key)
	{
        dataIterator it = data.find(key);
		if(it == data.end()){
            pairType* p = new pairType(key, valueType());
            keys.emplace_back(ptrType(p));
            data.emplace_hint(data.end(), dataType(key, p));
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
		if(it == data.end()){
			return 0;
		}
		pairType * p = (*it).second.get();
		for(keyIterator it2 = keys.begin(); it2 != keys.end(); ++it2){
			if((*it2).get() == p){
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
        if(it == data.end()){
            return keys.end();
        }
        pairType * p = (*it).second;
		for(keyIterator keyIt = keys.begin(); keyIt != keys.end(); ++keyIt){
			if((*keyIt).get() == p){
				return keyIt;
			}
		}
		return keys.end();
	}

	iterator erase(iterator it)
	{
		data.erase((*it).first);
		return keys.erase(it.real());
	}

	iterator erase(iterator &start, iterator &finnish)
	{
		for(keyIterator keyIt = start.real(); keyIt != finnish.real(); ++keyIt){
			data.erase((*keyIt)->first);
		}
		
		return keys.erase(start.real(), finnish.real());
	}

	bool operator==(const arbitrary_order_map& rhs) const
	{
		return data == rhs.data;
	}

	bool operator!=(const arbitrary_order_map& rhs) const
	{
		return data != rhs.data;
	}

	bool operator<=(const arbitrary_order_map& rhs) const
	{
		return keys <= rhs.keys;
	}

	bool operator>=(const arbitrary_order_map& rhs) const
	{
		return keys >= rhs.keys;
	}

	bool operator>(const arbitrary_order_map& rhs) const
	{
		return keys > rhs.keys;
	}

	bool operator<(const arbitrary_order_map& rhs) const
	{
		return keys < rhs.keys;
	}

	iterator insert(iterator at, pairType  val)
	{
		if (data.find(val.first) != data.end()) {
			for (auto it = keys.begin(); it != keys.end(); ++it) {
				if ((*it)->first == val.first) {
					(*it)->second = val.second;
					return it;
				}
			}
		}
		pairType* n = (new pairType(val));
		iterator it = keys.insert(at.real(), ptrType(n));
		data[val.first] = n;
		return it;
	}

	iterator insert(iterator start, iterator finnish)
	{
		iterator insIt = end();
		for(keyIterator keyIt = start.real(); keyIt != finnish.real(); ++keyIt){
			insIt = insert(end(), *(*keyIt));
		}
		return insIt;
	}

	iterator insert(iterator at, iterator start, iterator finnish)
	{
		iterator insIt = at;
		for(keyIterator keyIt = start.real(); keyIt != finnish.real(); ++keyIt){
			insIt = insert(insIt, *(*keyIt));
			++insIt;
		}
		return insIt;
	}
protected:
	std::map<keyType, pairType*>		data;
	std::vector<ptrType> 			keys;
};

#endif

#ifndef _ARBITRARY_ORDER_MAP_
#define _ARBITRARY_ORDER_MAP_

#include <map>
#include <deque>
#include <memory>

template<class keyType, class valueType, class A = std::allocator<valueType> >
class arbitrary_order_map
{
public:
    class pairType
    {
    public:
        pairType(const pairType & in) : first(in.first), second(in.second) {}
        pairType(const keyType & in) : first(in) {}
        pairType(const keyType & in, const valueType & inv) : first(in), second(inv) {}
        pairType(const std::pair<keyType, valueType> & pair) : first(pair.first), second(pair.second) {}
        keyType first;
        valueType second;
    };
//	typedef  std::pair<keyType, valueType> pairType;
	typedef  std::shared_ptr<pairType> ptrType;

	arbitrary_order_map(){}

	~arbitrary_order_map(){}

	valueType &operator[](const keyType &key)
	{
        typename std::map<keyType, ptrType>::iterator it = data.find(key);
		if(it == data.end()){
			;
            
            return (*(*(keys.emplace(keys.end(),
                                     data.insert(std::pair<keyType, ptrType>(key,
                                                                             ptrType(new pairType(key)))).first->second)))).second;
//			data.emplace(key, keys.back());
//            return t.get()->second;
		}
		return (*(*it).second).second;
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
        typename std::map<keyType, ptrType>::iterator it = data.find(key);
		if(it == data.end()){
			return 0;
		}
		pairType * p = (*it).second.get();
		for(typename std::deque<ptrType>::iterator it2 = keys.begin(); it2 != keys.end(); ++it2){
			if((*it2).get() == p){
				keys.erase(it2);
				break;
			}
		}

		return data.erase(key);
	}

	class iterator : public std::iterator<std::input_iterator_tag, std::pair<keyType, ptrType>>
	{
	public:
		friend class reverse_iterator;
		friend class const_iterator;
		iterator()
		{

		}

		iterator(const typename std::deque<ptrType>::iterator & in) : it(in)
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

		typename std::deque<ptrType>::iterator &real() 
		{
			return it;
		}
		
	public:
		typename std::deque<ptrType>::iterator 	it;
	};

		class const_iterator : public std::iterator<std::input_iterator_tag, std::pair<keyType, ptrType>>
	{
	public:
		friend class reverse_iterator;
		const_iterator()
		{

		}

		const_iterator(const typename std::deque<ptrType>::const_iterator & in) : it(in)
		{

		}

		const_iterator(typename std::deque<ptrType>::iterator & in) : it(in)
		{

		}

		const_iterator(const const_iterator& in) : it(in.it)
		{

		}

		const_iterator(const iterator& in) : it(in.it)
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
			return (*it).get();
		}

		const pairType *operator->()
		{
			return (*it).get();
		}

		const typename std::deque<ptrType>::const_iterator &real() 
		{
			return it;
		}
		
	public:
		typename std::deque<ptrType>::const_iterator 	it;
	};

	class reverse_iterator : public std::iterator<std::input_iterator_tag, std::pair<keyType, ptrType>>
	{
	public:
		reverse_iterator()
		{

		}

		reverse_iterator(const typename std::deque<ptrType>::reverse_iterator & in) : it(in)
		{

		}

		reverse_iterator(const reverse_iterator& in) : it(in.it)
		{

		}

		reverse_iterator(const iterator& in) : it(typename std::deque<ptrType>::reverse_iterator(in.it))
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

		typename std::deque<ptrType>::reverse_iterator &real() 
		{
			return it;
		}

	public:
		typename std::deque<ptrType>::reverse_iterator 	it;
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
        typename std::map<keyType, ptrType>::iterator it = data.find(key);
        if(it == data.end()){
            return keys.end();
        }
        pairType * p = (*it).second.get();
		for(typename std::deque<ptrType>::iterator keyIt = keys.begin(); keyIt != keys.end(); ++keyIt){
			if((*keyIt).get() == p){
				return keyIt;
			}
		}
		return keys.end();
	}

	void erase(iterator it)
	{
		data.erase((*it).first);
		keys.erase(it.real());
	}

	void erase(iterator &start, iterator &finnish)
	{
		for(typename std::deque<ptrType>::iterator keyIt = start.real(); keyIt != finnish.real(); ++keyIt){
			data.erase((*keyIt)->first);
		}
		
		keys.erase(start.real(), finnish.real());
	}

	bool operator==(const arbitrary_order_map& rhs) 
	{
		return data == rhs.data;
	}

	bool operator!=(const arbitrary_order_map& rhs) 
	{
		return data != rhs.data;
	}

	bool operator<=(const arbitrary_order_map& rhs) 
	{
		return data <= rhs.data;
	}

	bool operator>=(const arbitrary_order_map& rhs) 
	{
		return data >= rhs.data;
	}

	bool operator>(const arbitrary_order_map& rhs) 
	{
		return data > rhs.data;
	}

	bool operator<(const arbitrary_order_map& rhs) 
	{
		return data < rhs.data;
	}

	iterator insert(iterator at, pairType  val)
	{
		ptrType n(new pairType(val));
		iterator it = keys.insert(at.real(), n);
		data[val.first] = n;
		return it;
	}

	void insert(iterator start, iterator finnish)
	{
		for(typename std::deque<ptrType>::iterator keyIt = start.real(); keyIt != finnish.real(); ++keyIt){
			insert(end(), *(*keyIt));
		}
	}
protected:
	std::map<keyType, ptrType>		data;
	std::deque<ptrType> 			keys;
};

#endif

#if !defined JSTRING_HPP
#define JSTRING_HPP

#include <limits>
#include <cstring>
#include <string>
// #if defined _WINDOWS && !defined __clang__ && _MSC_VER >= 1900
// #define noexcept
// #endif
template <class T>
struct secure_delete_allocator {
	typedef T value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;

	size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(value_type); }
	secure_delete_allocator() noexcept {}
	template <class U> secure_delete_allocator (secure_delete_allocator<U> const&) noexcept {}
	value_type* allocate (std::size_t n) { return static_cast<value_type*>(malloc(n*sizeof(value_type))); }
	void deallocate (value_type* p, std::size_t n) { memset((void*)p, 0, n); free(p); }
	template <class U> struct rebind { typedef secure_delete_allocator<U> other; };

//	void construct(pointer p, const value_type& val)
//	{
//		new(static_cast<void*>(p)) value_type(val);
//	}

//	void construct(pointer p)
//	{
//		new(static_cast<void*>(p)) value_type();
//	}

//	void destroy(pointer p)
//	{
//		p->~value_type();
//	}

};

#if defined USE_STD_STRING
#define sdstring std::string
#else


template <class T, class U>
constexpr bool operator== (const secure_delete_allocator<T>&, const secure_delete_allocator<U>&) noexcept
{return true;}

template <class T, class U>
constexpr bool operator!= (const secure_delete_allocator<T>&, const secure_delete_allocator<U>&) noexcept
{return false;}

typedef std::basic_string<char, std::char_traits<char>, secure_delete_allocator<char> > base_sdstring;

class sdstring : public base_sdstring
{
	public:
		sdstring(const sdstring & rhs) : base_sdstring(rhs.data(), rhs.size()) { }

		sdstring(sdstring && rhs) : base_sdstring(std::move(rhs)) { }

		sdstring(const std::string & rhs) : base_sdstring(rhs.data(), rhs.size()) { }

		sdstring(std::string && rhs) : base_sdstring(std::move(rhs.data()), rhs.size()) { }

		sdstring(const char* rhs, size_t size) : base_sdstring(rhs, size) { }
		sdstring() : base_sdstring() {}

		~sdstring() {  }

		using base_sdstring::base_sdstring;
		using base_sdstring::operator[];
		using base_sdstring::operator=;
//		using base_sdstring::operator<;
		using base_sdstring::operator+=;
		using base_sdstring::append;
		using base_sdstring::assign;
		using base_sdstring::size;
		using base_sdstring::find;
		size_t find(const std::string& in, size_t pos = 0) const { return find(*in.c_str(), pos); }

        sdstring& operator=(const sdstring& rhs)
        {
            if (&rhs != this) {
                static_cast<base_sdstring&>(*this) = static_cast<const base_sdstring&>(rhs);
            }
            return *this;
        }

        sdstring& operator=(sdstring&& rhs)
        {
            if (&rhs != this) {
                static_cast<base_sdstring&>(*this) = static_cast<const base_sdstring&&>(rhs);
            }
            return *this;
        }
//
        operator std::string&() const
        {
            return *((std::string*) this);
        }
//
//    operator const std::basic_string<char>&()
//        {
//            return *((const std::string*) this);
//        }

//     operator std::string() const
//    {
//        return std::string(this->c_str());
//    }
    
//     operator const std::string() const
//    {
//        return std::string(this->c_str());
//    }
    
        bool operator==(const sdstring& rhs) const
        {
            return (rhs == this->c_str());
        }

        bool operator!=(const sdstring& rhs) const
        {
            return (rhs != this->c_str());
        }

        bool operator==(const char* rhs) const
        {
            return (reinterpret_cast<const std::string&>(*this) == rhs);
        }

        bool operator!=(const char* rhs) const
        {
            return (rhs != this->c_str());
        }

//		size_t operator<(const sdstring& rhs) const
//		{
//			return std::less<std::string>{}(static_cast<std::string&>(*this), static_cast<std::string&>(rhs));
//		}

//        sdstring& append(const std::string& rhs)
//        {
//            this->append(rhs.c_str());
//            return *this;
//        }
};
sdstring operator+(const sdstring & lhs, const std::string & rhs);
sdstring operator+(const sdstring & lhs, const sdstring & rhs);
sdstring operator+(const char*, const sdstring & rhs);
sdstring operator+(const sdstring & lhs, const char* rhs);
std::string operator+(const std::string & lhs, const sdstring & rhs);
bool operator==(const std::string & lhs, const sdstring & rhs);
bool operator!=(const std::string & lhs, const sdstring & rhs);
//size_t operator<(sdstring const& s, sdstring const& s2)
//{
//	return std::less<std::string>{}(static_cast<std::string&>(s), static_cast<std::string&>(s2));
//}

// struct SDStringComparator
// {
// 	bool
// 	operator()(const sdstring & obj1, const sdstring & obj2) const
// 	{
// 		return (obj1 == obj2);
// 	}
// };

namespace std
{
template<>
	struct hash<sdstring>
	{
        typedef sdstring argument_type;
        typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
            return std::hash<std::string>{}(std::string(s.c_str()));
		}
	};
template<>
	struct less<sdstring>
	{
        typedef sdstring argument_type;
        typedef std::size_t result_type;
		result_type operator()(argument_type const& s, argument_type const& s2) const
/*        typedef bool result_type;
        result_type operator()(argument_type const& s, argument_type const& s2) const NOEXCEPT*/
		{
            return std::less<std::string>{}(static_cast<std::string&>(s), static_cast<std::string&>(s2));
		}
	};
}
#endif
#endif //JSTRING_HPP

#pragma once

#include <vector>
#include <algorithm>

template<typename T>
class Set{
public:
	typedef typename std::vector<T>::const_iterator CSetIterator;

	CSetIterator begin( ) const {
		return m_items.cbegin( );
	}

	CSetIterator end( ) const {
		return m_items.cend( );
	}

	size_t size( ) const {
		return m_items.size( );
	}

	bool put( const T& item ){
		auto it = std::find_if( m_items.begin( ), m_items.end( ), [&item]( const T& it ){return it == item; } );

		if( it != m_items.end( ) )
			return false;

		m_items.push_back( item );
		return true;
	}

	int insert( const T& item ){
		auto it = std::find_if( m_items.begin( ), m_items.end( ), [&item]( const T& it ){return it == item; } );

		if( it != m_items.end( ) )
			return it - m_items.begin( );

		m_items.push_back( item );
		return m_items.size( ) - 1;
	}

	bool empty( ) const {
		return m_items.empty( );
	}

	bool contains( const T& item ) const {
		return std::find( m_items.begin( ), m_items.end( ), item ) != m_items.end( );
	}

	bool operator==(const Set<T>& other) const {
		if( m_items.size( ) != other.size( ) )
			return false;

		auto vec = m_items;
		auto last = vec.end( );

		for( auto && it : other.m_items ){
			auto newLast = std::remove( vec.begin( ), last, it );

			if( last == newLast )
				return false;
			else
				last = newLast;
		}

		return vec.begin( ) == last;
	}

	const T& operator[]( size_t index ) const {
		return m_items[index];
	}

	Set<T>& operator-=(const Set<T>& rhs){
		for( auto && item : rhs.m_items ){
			m_items.erase( std::remove( m_items.begin( ), m_items.end( ), item ), m_items.end( ) );
		}
		return *this;
	}

	Set<T>& operator+=(const Set<T>& rhs){
		for( auto && item : rhs.m_items ){
			put( item );
		}
		return *this;
	}

	friend Set<T> operator+(Set<T> lhs, const Set<T>& rhs){
		auto result = lhs;
		result += rhs;
		return result;
	}

	friend Set<T> operator-(Set<T> lhs, const Set<T>& rhs){
		auto result = lhs;
		result -= rhs;
		return result;
	}

	Set<T>& operator=(const Set<T>& rhs){
		if( this != &rhs )
			m_items = rhs.m_items;

		return *this;
	}

	Set( ){};
	Set( const Set<T>& cpy )
		: m_items( cpy.m_items )
	{}
	Set( std::initializer_list<T> init )
		: m_items( init )
	{}

private:
	std::vector<T> m_items;
};
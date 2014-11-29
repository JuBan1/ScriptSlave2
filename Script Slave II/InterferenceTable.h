#ifndef INTERFERENCE_TABLE
#define INTERFERENCE_TABLE

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

template<typename C>
void erase_nth( C& c, size_t n ){
	c.erase( c.begin( ) + n );
}

class InterferenceTable{
public:

	void RemoveEntry( size_t rowOrCol ){
		erase_nth( m_labels, rowOrCol );
		erase_nth( m_table, rowOrCol );
		for( auto&& v : m_table )
			erase_nth( v, rowOrCol );
	}

	std::string GetLabel( size_t rowOrCol ){
		return m_labels[rowOrCol];
	}
	void SetLabel( std::string name, size_t rowOrCol ){
		m_labels[rowOrCol] = name;
	}

	std::vector<bool>& operator[]( size_t x ){
		return m_table[x];
	}

	void Print( ) const {
		for( auto&& n : m_labels )
			std::cout << n << " ";
		std::cout << "\n";
		for( auto&& x : m_table ){
			for( auto&& y : x )
				std::cout << y << " ";
			std::cout << "\n";
		}
		std::cout << "\n";
	}

	void Reduce( ){
		const size_t startSize = m_table.size( );
		for( int x = m_table.size( ) - 1; x >= 0; --x ){
			int count = 0;
			for( size_t i = 0; i < m_table.size( ); ++i )
			if( m_table[x][i] == true )
				++count;

			if( count == 0 )
				RemoveEntry( x );
			else if( count == m_table.size( ) ){
				RemoveEntry( x );
				m_numOfMaxEdgeVars += 1;
			}
		}

		if( m_table.size( ) < startSize ) //Try to repeatedly reduce until no changes occur.
			Reduce( );
	}

	InterferenceTable( size_t size )
		: m_table( size ), m_labels( size, "?" ), m_numOfMaxEdgeVars( 0 )
	{
		for( auto&& e : m_table )
			e.resize( size );
	}

	int FindMinVarNum( ){
		const int size = m_table.size( );

		if( size > 8 ) //Run time just too high for a count of 9+
			return m_numOfMaxEdgeVars + size;


		std::vector<int> comb( size );

		for( int i = 1; i < size; ++i ){
			if( n_over_k( 0, i, size, comb ) ){
#ifdef DEBUG_BUILD
				for( int i : comb ){
					std::cout << i << " ";
				}
				std::cout << "\n";
#endif
				return i + m_numOfMaxEdgeVars;
			}
		}

		//Can't combine any more variables.
		return size + m_numOfMaxEdgeVars;
	}

private:

	bool MatchResult( const std::vector<int>& combination ){
		const size_t size = m_table.size( );

		for( size_t x = 0; x < size; ++x )
		for( size_t y = 0; y < x; ++y )
		if( combination[x] == combination[y] && m_table[x][y] )
			return false;
		return true;
	}

	bool n_over_k( int n, int maxN, int size, std::vector<int>& combination ){
		if( n == size ){
			return MatchResult( combination ); //Return if matching successful
		}

		for( int i = 0; i < maxN; ++i ){
			combination[n] = i;
			if( n_over_k( n + 1, maxN, size, combination ) )
				return true;
		}
		return false;
	}


	std::vector<std::string> m_labels;
	std::vector<std::vector<bool>> m_table;
	unsigned int m_numOfMaxEdgeVars;
};

#endif
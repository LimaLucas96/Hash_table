/*
	@AUTHOR: Lucas Lima M. de Oliveira
*/
#ifndef HASH_TABLE_H
#define HASH_TABLE_H


#include <string.h>
#include <vector>

#include <iostream>     // cout, endl, ostream
#include <forward_list> // forward_list
#include <algorithm>    // copy, find_if, for_each
#include <cmath>        // sqrt
#include <iterator>     // std::begin(), std::end()
#include <initializer_list> //std::initializer_list
#include <utility> // std::pair
namespace ac{	
	template < typename KeyType,
             typename DataType,
             typename KeyHash = std::hash<KeyType>,
             typename KeyEqual = std::equal_to<KeyType> >
    class HashTbl
    {
        class HashEntry
        {
            public:
                HashEntry( KeyType k_, DataType d_) : 
                m_key( k_ ),
                m_data ( d_ ){ /*empty*/ };

                KeyType m_key;
                DataType m_data;

                KeyType getKey()const{ return m_key; }
                friend std::ostream &operator<<( std::ostream &os_, const HashEntry& h_){
                    return os_ << h_.m_key << " , " << h_.m_data;
                }
        };
        
        public:

            using Entry = HashEntry; //<! Alias
           
            //[I]SPECIAL MEMBERS
            
            explicit HashTbl(size_t tbl_size_ = DEFAULT_SIZE )
            :m_size{NextPrime(tbl_size_)},
            m_count(0){
                Lists.resize(NextPrime(tbl_size_));
                //m_data_table[0];
            }
            
            HashTbl( const HashTbl &other ){
                m_size = other.Lists.size();
                m_count = 0;
                Lists.resize(m_size);

                for(auto list(other.Lists.cbegin()); list != other.Lists.cend();list++){
                	for(auto i(list->cbegin()); i != list->cend();i++){
                		insert(i->m_key,i->m_data);
                	}
                }
            }

            virtual ~HashTbl(){ /*empt*/ }

            template <typename InputItr>
            HashTbl(InputItr first , InputItr last ){
                m_size = NextPrime(std::distance(first,last));
                Lists.resize(m_size);
                for(auto list(first); list != last; list++){
                   for(auto i(list->cbegin()); i != list->cend();i++){
                		insert(i->m_key,i->m_data);
                	}
                }
            }

            HashTbl( std::initializer_list<Entry> ilist){
            	m_size = NextPrime(ilist.size());
            	Lists.resize(m_size);

            	for(auto &i:ilist){
            		insert(i.m_key, i.m_data);
            	}
            }
           	
           	//[II]OPERADORS
            
            HashTbl& operator=( const HashTbl& k_){  
            	if(m_size != 0){
            		clear();

            		m_size = k_.Lists.size();
           			Lists.resize(m_size);

               		for(auto list(k_.Lists.cbegin()); list != k_.Lists.cend();list++){
                		for(auto i(list->cbegin()); i != list->cend();i++){
                			insert(i->m_key,i->m_data);
                		}
                	}
                }

                return *this;
            }

            HashTbl& operator=( std::initializer_list<Entry> ilist ){
            	if(m_size != 0){
            		m_size = NextPrime(ilist.size());

            		for(auto &i:ilist){
            			insert(i.m_key, i.m_data);
            		}
            	}

            	return *this;
            }

            DataType& operator[]( const KeyType & k_ ){
                auto &list = Lists[ Hfunction( k_ ) % m_size ];
                auto itr = list.begin();
                auto end = list.end();
            	DataType d(0);

            	for(/* */;itr != end; itr++){
                    if( Efunction(itr->m_key, k_) ){
                    	//DataType &ret = itr->m_data;
                        return itr->m_data;
                    }
                }

                insert(k_,d);
                
                return at(k_);
            }

            //[III]MODIFIERS
            

            bool insert(const KeyType & k_ , const DataType & d_ ){
                auto &list = Lists[ Hfunction( k_ ) % m_size ];
                Entry NEntry( k_, d_ );
                auto itr = list.begin();                            //!< Iterator to the List's begin
                auto itr_b = list.before_begin();                   //!< Iterator to the position before the list's begin 
                auto end = list.end();                          //!< Iterator to the list's end
                for ( /* */; itr != end; ++itr ){
                    itr_b++;
                    if ( Efunction(itr->m_key, NEntry.m_key) ){
                        list.insert_after(itr, NEntry);
                        erase(itr->m_key);
                        return false;
                    }
                }

                list.push_front( NEntry );
                m_count++;
                if( m_count > m_size ) 
                    rehash();

                return true; 
            }

            bool erase( const KeyType & k_ ){
                auto &list = Lists[Hfunction(k_) % m_size];
                auto itr = list.before_begin();

                for(auto i = list.begin(); i != list.end(); i++){
                    if(Efunction(i->m_key , k_ )){
                        list.erase_after(itr);
                        m_count --;
                        return true;
                    }
                    itr ++;
                }
                return false;
            }

            bool retrieve ( const KeyType & k_ , DataType & d_ ) const{

                auto &list = Lists[Hfunction(k_)%m_size];
                //int cont(0);
                for(auto i(list.begin()) ; i != list.end(); i++){
                    //cont++;
                    if( Efunction(i->m_key , k_ ) ){
                        //std::cout << "😿 = " << ++cont <<"\n";
                        d_ = i->m_data;
                        return true;
                    }
                }
                return false;
            }


            //[IV]CAPACITY
            
            void clear( void ){ m_count = 0; for(auto i(Lists.begin());i != Lists.end(); i++){ i->clear(); } }
            bool empty( void ) const{ return m_count == 0; }
            inline size_t size() const { return m_count; }

            DataType& at( const KeyType & k_ ){
                auto &list = Lists[ Hfunction( k_ ) % m_size ];
                auto itr = list.begin();
                auto end = list.end();

                for(/* */;itr != end; itr++){
                    if( Efunction(itr->m_key, k_) ){
                        DataType &ret = itr->m_data;
                       // std::cout << ret<<" AQUIIIIII\n";
                        
                        return ret;
                    }
                }
                throw std::out_of_range("Key not found.");
            }

            size_t count( const KeyType & k_ ) const{
            	auto &list = Lists[ Hfunction( k_ ) % m_size ];
                auto itr = list.begin();
                auto end = list.end();
                size_t cont(0);
                for(/* */;itr != end; itr++){
                    cont++;
                }

                return cont;
            }

            friend std::ostream &operator<<( std::ostream &os_, const HashTbl& h_){
                int cont(0);
                for(auto list(h_.Lists.cbegin()); list != h_.Lists.cend(); list++){
                    if(not list->empty()){
                        os_ << "[" << cont <<"]->\n";
                        for(auto i((*list).cbegin()); i != (*list).cend(); i++){
                            //os_ << "key {" << i->m_key << "} , data { " << (*i).m_data << " }\n";

                            os_ << (*i).m_data<<"\n";
                        }
                            
                    }else{
                        os_ << "[" << cont <<"]-> \"Empty\"\n";
                    }

                    cont++;
                    //std::copy(list->cbegin(),list->cend(), std::back_insert_iterator< HashEntry > o());
                }

            return os_;
            }

        private:
            void rehash(){

                std::vector< std::forward_list<Entry> > oldLists = Lists;
    
                /* Creates a new double-sized empty table */
                m_size = NextPrime( m_size * 2 );
    
                Lists.resize( m_size );
                m_count = 0;

                for( auto & thisList : Lists )
                    thisList.clear(); 

                    /* Copies table over */
                for( auto & copyLists : oldLists )
                    for( auto & x : copyLists )
                        insert( x.m_key, x.m_data );
            }

            int NextPrime( int number ){
                for( size_t i = 2; i <= sqrt( number ); ++i ) {
                    if(number % i == 0){
                        //std::cout << " 🤟 -> " << number << " | " << sqrt(number) << "\n";
                        return NextPrime( number + 1 );
                    }
                }
                return number;
            }

            int m_size;
            unsigned int m_count;

            KeyHash Hfunction;
            KeyEqual Efunction;

            using ListEntry = std::forward_list< Entry > ;
            std::vector< ListEntry > Lists;

            static const short DEFAULT_SIZE = 11;
            
    };
}

#endif
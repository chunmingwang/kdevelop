/***************************************************************************
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HASHED_STRING_H
#define HASHED_STRING_H

#include<QString>
#include<ksharedptr.h>
#include<set>
#include <ext/hash_map>
#include <string>
#include "cppparserexport.h"

class QDataStream;
typedef uint HashType; ///@todo use at least 64 bit hash, if not 128 bit


///A simple class that stores a string together with it's appropriate hash-key
class KDEVCPPPARSER_EXPORT HashedString {
  public:
    HashedString() : m_hash( 0 ) {}

    HashedString( const QString& str ) : m_str( str ) {
      initHash();
    }
    
    HashedString( const char* str ) : m_str( str ) {
      initHash();
    }

    inline HashType hash() const {
      return m_hash;
    }

    QString str() const {
      return m_str;
    }

    bool operator == ( const HashedString& rhs ) const {
      if ( m_hash != rhs.m_hash )
        return false;
      return m_str == rhs.m_str;
    }

    ///Does not compare alphabetically, uses the hash-key for ordering.
    bool operator < ( const HashedString& rhs ) const {
      if ( m_hash < rhs.m_hash )
        return true;
      if ( m_hash == rhs.m_hash )
        return m_str < rhs.m_str;
      return false;
    }

    static HashType hashString( const QString& str );

  private:
    void initHash();

    QString m_str;
    HashType m_hash;

    friend QDataStream& operator << ( QDataStream& stream, const HashedString& str );
    friend QDataStream& operator >> ( QDataStream& stream, HashedString& str );
};

QDataStream& operator << ( QDataStream& stream, const HashedString& str );

QDataStream& operator >> ( QDataStream& stream, HashedString& str );

class HashedStringSetData;
class HashedStringSetGroup;

///This is a reference-counting string-set optimized for fast lookup of hashed strings
class KDEVCPPPARSER_EXPORT HashedStringSet {
  public:
    HashedStringSet();

    ~HashedStringSet();

    ///Constructs a string-set from one single file
    HashedStringSet( const HashedString& file );

    HashedStringSet( const HashedStringSet& rhs );

    int size() const;

    HashedStringSet& operator = ( const HashedStringSet& rhs );
    ///@return whether the given file-name was included
    bool operator[] ( const HashedString& rhs ) const;

    void insert( const HashedString& str );

    HashedStringSet& operator +=( const HashedStringSet& );
    
    HashedStringSet& operator -=( const HashedStringSet& );

    ///intersection-test
    ///Returns true if all files that are part of this set are also part of the given set
    bool operator <= ( const HashedStringSet& rhs ) const;

    bool operator == ( const HashedStringSet& rhs ) const;

    void read( QDataStream& stream );
    void write( QDataStream& stream ) const;

    std::string print() const;

  HashType hash() const;
  private:
    friend class HashedStringSetGroup;
    void makeDataPrivate();
    KSharedPtr<HashedStringSetData> m_data; //this implies some additional cost because KShared's destructor is virtual. Maybe change that by copying KShared without the virtual destructor.
    friend HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );
};

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );

namespace __gnu_cxx {
template<>
struct KDEVCPPPARSER_EXPORT hash<HashedString> {
  HashType operator () ( const HashedString& str ) const {
    return str.hash();
  }
};
}

///Used to find all registered HashedStringSet's that contain all strings given to findGroups(..)
class KDEVCPPPARSER_EXPORT HashedStringSetGroup {
  public:
    typedef std::set<unsigned int> ItemSet;
    void addSet( unsigned int id, const HashedStringSet& set );
    void enableSet( unsigned int id );
    bool isDisabled( unsigned int id ) const;
    void disableSet( unsigned int id );
    void removeSet( unsigned int id );

    //Writes the ids of all registered and not disabled HashedStringSet's that are completely included in the given HashedStringSet efficiently)
    void findGroups( HashedStringSet strings, ItemSet& target ) const;

  private:
    typedef __gnu_cxx::hash_map<HashedString, ItemSet> GroupMap;
    typedef __gnu_cxx::hash_map<unsigned int, unsigned int> SizeMap;
    GroupMap m_map;
    SizeMap m_sizeMap;
    ItemSet m_disabled;
    ItemSet m_global;
};

class HashedStringSubset;

class KDEVCPPPARSER_EXPORT HashedStringRepository {
  public:
    typedef __gnu_cxx::hash_map<HashedString, HashedStringSubset*> AtomicSubsetMap;
    typedef __gnu_cxx::hash_map<HashType, HashedStringSubset*> HashMap;

    HashedStringSubset* getAtomicSubset( const HashedString& str );

    /**
     * Takes a list of atomic sub-sets(hashed-strings), and construct a master-subset
     * The returned set will be registered in the repository
     * @param atomics list of atomic subsets
     * @return the created set, or 0 on failure
     * */
    HashedStringSubset* buildSet( const QList<HashedStringSubset*> atomics );

    /**
     * @return a HashedStringSubset that represents the union of the given sets.
     * */
    HashedStringSubset* merge( HashedStringSubset* left, HashedStringSubset* right );

    /**
     * @return intersection of left and right
     * */
    HashedStringSubset* intersect( HashedStringSubset* left, HashedStringSubset* right );
    
    void dump( HashedStringSubset* subset );

    QString dumpDotGraph();
  private:
    //After a new HashedStringSubset was created, this must be called to correctly connect it correctly to it's master-sets
    void connectToMasterSets( HashedStringSubset* set );
    AtomicSubsetMap m_atomicSubsets;
    HashMap m_allSubsets;
};

#endif

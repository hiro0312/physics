#pragma once

#include <cstring>
#include <cassert>

template <typename T>
struct ArrayFreeList
{
    ArrayFreeList( int capacity = 128 )
    {
        m_elements = new T[capacity];
        m_nextFreeIds = new int[capacity];
        m_firstFreeId = 0;
        m_numElements = 0;
        m_capacity = capacity;

        for ( int i = 0; i < m_capacity; i++ )
        {
            m_nextFreeIds[i] = i + 1;
        }
    }

    ~ArrayFreeList()
    {
        delete[] m_elements;
        delete[] m_nextFreeIds;
    }

    // Get number of used spaces among allocated spaces for elements
    int getSize() { return m_numElements; }

    // Get number of allocated spaces for elements
    int getCapacity() { return m_capacity; }

    // Return if particular index is used
    bool isUsed( int index ) { return (m_nextFreeIds[index] == -1); }

    // Return if buffer is currently full and requires expansion
    bool isFull() { return (m_firstFreeId == m_capacity); }

    T& operator()( int index )
    {
        assert( isUsed( index ) );
        return m_elements[index];
    }

    int add( const T& t )
    {
        // Not expanding automatically here for renderer to handle
        assert( !isFull() );

        int thisId = m_firstFreeId;

        T& element = m_elements[thisId];
        element = t;

        m_firstFreeId = m_nextFreeIds[thisId];
        m_nextFreeIds[thisId] = -1;

        m_numElements++;

        return thisId;
    }

    void remove( int index )
    {
        m_nextFreeIds[index] = m_firstFreeId;
        m_firstFreeId = index;

        m_numElements--;
    }

    void expand()
    {
        // Make new storage /w double capacity, copy from old
        T* newElements = new T[m_capacity * 2];
        memcpy( newElements, m_elements, m_capacity * sizeof( T ) );
        delete[] m_elements;
        m_elements = newElements;

        int* newNextFreeIds = new int[m_capacity * 2];
        memcpy( newNextFreeIds, m_nextFreeIds, m_capacity * sizeof( int ) );
        delete[] m_nextFreeIds;
        m_nextFreeIds = newNextFreeIds;

        // Append expanded free Ids
        m_firstFreeId = m_capacity;
        for ( int i = m_firstFreeId; i < m_capacity * 2; i++ )
        {
            m_nextFreeIds[i] = i + 1;
        }

        m_capacity *= 2;
    }

private:

    T * m_elements;
    int* m_nextFreeIds;
    int m_firstFreeId;

    int m_numElements;
    int m_capacity;
};

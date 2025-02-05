#ifndef __MAP_H__
#define __MAP_H__

// class static_map<>
// ==================
template <class TKey, class Ty>
class static_map 
{
public:
    class _elem
    {
    public:
        TKey first;
        Ty second;

        _elem(TKey _first, Ty _second) 
            : first(_first), second(_second) 
        {}
    };

    typedef TKey key_type;
    typedef _elem value_type;
    typedef const value_type* const_iterator;
    typedef static_map<TKey,Ty> this_type;

    static_map(const_iterator _begin, const_iterator _end)
        : m_pBegin(_begin), m_pEnd(_end) { ASSERT(_end>=_begin); }

    static_map(const this_type& sm)
        : m_pBegin(sm.m_pBegin), m_pEnd(sm.m_pEnd) {}

    const_iterator begin() const { return m_pBegin; }
    const_iterator end() const { return m_pEnd; }

    const Ty operator[](key_type key) const
    {
        for(const_iterator itr=begin();
            itr!=end();
            itr++)
            if (itr->first==key)
                return itr->second;        
        return (Ty) NULL;
    }
protected:
    const_iterator m_pBegin, m_pEnd;
};


#endif
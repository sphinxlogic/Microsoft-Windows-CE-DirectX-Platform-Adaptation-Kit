
//--------------------------------------------------------------------------;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 2000 Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
// File: com_utilities.h
// Template Library for COM
//
// contains: 
//     class CComPointer<>
//     macro SafeQI
//     macro SafeQI_
//     function UUIDOF
//--------------------------------------------------------------------------;


#pragma once

#ifndef __COM_UTILITIES_H__
#define __COM_UTILITIES_H__

// UUIDOF
// ======
template <class TInterface>
inline REFIID UUIDOF(TInterface **p)
{
    return __uuidof(**p);
}

#define DECLARE_STD_UUIDOF_SPECIALIZATION(INTERFACE) template <> inline REFIID UUIDOF(INTERFACE **p) { return IID_##INTERFACE; }

// A Safer QueryInterface 
// ======================
#define SafeQI(EXPR) QueryInterface(UUIDOF(EXPR), reinterpret_cast<void**>(EXPR)) 
#define SafeQI_(INTERFACE,EXPR) QueryInterface(__uuidof(**(EXPR)), reinterpret_cast<void**>(static_cast<INTERFACE**>(EXPR)))

// class CComPointerBase
//    Base for smart pointer class for COM interfaces.
//    Automatically handles AddRef and Release
// ===================================================
template <class TInterface>
class CComPointerBase
{
    friend class CStabilize;
protected:
    TInterface *m_pi;

    // constructor is hidden to prevent 
    // direct instances of this class
    CComPointerBase() : m_pi(NULL) {}

public:
    typedef TInterface interface_type;

    // Access Operator (dummy class hides AddRef and Release
    // but, unfortunately, confuses code-completion feature of DevStudio 6)
    // --------------------------------------------------------------------
    class _TInterface : public TInterface
    {
    private:
        virtual ULONG STDMETHODCALLTYPE AddRef() = 0;    
        virtual ULONG STDMETHODCALLTYPE Release() = 0;
    };
    
    _TInterface* operator -> () const 
    {
        return static_cast<_TInterface*>(m_pi);
    }

    // ReleaseInterface
    // ----------------
    void ReleaseInterface()
    {
        if (m_pi!=NULL)
            m_pi->Release();
        m_pi = NULL;
    }

    // Destructor
    // ----------
    virtual ~CComPointerBase() 
    { 
        if (m_pi!=NULL) 
            m_pi->Release(); 
    }

    // predicate members
    // -----------------
    class _tag {};

    operator _tag* () const 
    {
        return reinterpret_cast<_tag*>(m_pi);
    }

    operator !() const 
    {
        return NULL == m_pi;
    }

    bool IsAssigned() const
    { 
        // kept for backward compatability
        return m_pi!=NULL; 
    }

    // members for retrieving the interface pointer
    // --------------------------------------------
    TInterface* GetInterface(BOOL bAddRef) const
    {  
        if (bAddRef && m_pi!=NULL)
            m_pi->AddRef();  
        return m_pi;
    }

    TInterface* AsInParam() const
    {
        return GetInterface(FALSE);
    }

    TInterface* AsRightHandSide() const
    {
        return GetInterface(TRUE);
    }

    TInterface** AsOutParam () 
    {
        if (m_pi!=NULL)
        {
            m_pi->Release();
            m_pi=NULL;
        }
        return &m_pi;
    }

    TInterface** AsInOutParam () 
    {
        return &m_pi;
    }

};



// class CComPointer
//    smart pointer class for COM interfaces.
//    Automatically handles AddRef and Release
// ===========================================
template <class TInterface>
class CComPointer : public CComPointerBase<TInterface>
{
public:
    typedef CComPointerBase<TInterface> base_class;
    typedef CComPointer<TInterface> this_type;
    typedef base_class::interface_type interface_type;

    // Constructors
    // ------------
    CComPointer() {}

    CComPointer(TInterface *pi)
    {
        m_pi=pi;
        if (pi!=NULL) m_pi->AddRef();
    }

    explicit CComPointer(IUnknown *punk)
    {
        if (punk!=NULL)
        {
            HRESULT hr = punk->SafeQI(&m_pi);
            ASSERT(SUCCEEDED(hr));
        }
        else
            m_pi=NULL;
    }

    explicit CComPointer(void *pv)
    {
        ASSERT(pv==NULL);
        m_pi = static_cast<TInterface*>(pv);
    }

    // Assignment Operators
    // --------------------
    this_type& operator = (interface_type *pi)
    {
        interface_type *tmp = m_pi;
        m_pi = pi;
        if (m_pi!=NULL) m_pi->AddRef();
        if (tmp!=NULL) tmp->Release();
        return *this;
    }

    this_type& operator = (const this_type& pi) 
    {
        interface_type *tmp = m_pi;
        m_pi = pi.m_pi;
        if (m_pi!=NULL) m_pi->AddRef();
        if (tmp!=NULL) tmp->Release();
        return *this;
    }
   
};


// class CComPointer<IUnknown>
//    Template specialization of CComPointer for IUnknown
// ======================================================
template <>
class CComPointer<IUnknown> : public CComPointerBase<IUnknown>
{
    friend class CStabilize;

public:
    
    // Constructors and Assignment Operators
    // -------------------------------------
    CComPointer() { } 

    explicit CComPointer(IUnknown *pi)
    {
        m_pi = pi;
        if (m_pi!=NULL) m_pi->AddRef();
    }

    explicit CComPointer(void *pv)
    {
        ASSERT(pv==NULL);
        m_pi = static_cast<IUnknown*>(pv);
    }

    CComPointer<IUnknown>& operator = (IUnknown *pi)
    {
        IUnknown *punk = m_pi;
        m_pi = pi;
        if (pi!=NULL) m_pi->AddRef();
        if (punk!=NULL) punk->Release();

        return *this;
    }

};


#endif
#ifndef HALIDE_INTRUSIVE_PTR_H
#define HALIDE_INTRUSIVE_PTR_H

/** \file
 *
 * Support classes for reference-counting via intrusive shared
 * pointers.
 */

#include <stdlib.h>

#include "Util.h"

namespace Halide {
namespace Internal {

/** A base class for reference-counted objects. */
class RefCounted {
public:
    mutable int ref_count = 0;
    virtual ~RefCounted() {}

    void incref() const {
        ref_count++;
    }

    void decref() const {
        // Note that if the refcount is already zero, then we're
        // in a recursive destructor due to a self-reference (a
        // cycle), where the ref_count has been adjusted to remove
        // the counts due to the cycle. The next line then makes
        // the ref_count negative, which prevents actually
        // entering the destructor recursively.
        ref_count--;
        if (ref_count == 0) {
            delete this;
        }
    }
};

/** Intrusive shared pointers have a reference count stored in the
 * class itself. This is perhaps more efficient than storing it
 * externally, but more importantly, it means it's possible to recover
 * a reference-counted handle from the raw pointer, and it's
 * impossible to have two different reference counts attached to the
 * same raw object. Seeing as we pass around raw pointers to concrete
 * IRNodes and Expr's interchangeably, this is a useful property.
 *
 * Anything for which there exists an incref and decref function can
 * be held by an IntrusivePtr. The simplest way to define these is to
 * inherit from RefCounted<T>.
 */
template<typename T>
struct IntrusivePtr {
    RefCounted *ptr;

    void incref(RefCounted *p) const {
        if (p) p->incref();
    }

    void decref(RefCounted *p) const {
        if (p) p->decref();
    }

public:

    T *get() const {
        return (T*)ptr;
    }
    
    ~IntrusivePtr() {
        decref(ptr);
    }

    IntrusivePtr() : ptr(NULL) {
    }

    IntrusivePtr(T *p) : ptr((RefCounted *)p) {
        incref(ptr);
    }

    IntrusivePtr(const IntrusivePtr<T> &other) : ptr(other.ptr) {
        incref(ptr);
    }
    
    IntrusivePtr<T> &operator=(const IntrusivePtr<T> &other) {
        // Other can be inside of something owned by this, so we
        // should be careful to incref other before we decref
        // ourselves.
        RefCounted *temp = other.ptr;
        incref(temp);
        decref(ptr);
        ptr = temp;
        return *this;
    }

    /** Access a property or method of the pointed-to object. */
    T *operator->() const {
        return (T*)ptr;
    }
    
    /* Handles can be null. This checks that. */
    bool defined() const {
        return ptr != NULL;
    }

    /* Check if two handles point to the same ptr. This is
     * equality of reference, not equality of value. */
    bool same_as(const IntrusivePtr &other) const {
        return ptr == other.ptr;
    }

    /** Define less than for intrusive pointers so that they can be
     * used in std::set, std::map, etc. */
    bool operator<(const IntrusivePtr &other) const {
        return ptr < other.ptr;
    }
};

}
}

#endif

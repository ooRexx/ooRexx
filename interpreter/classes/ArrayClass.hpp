/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* REXX Kernel                                               ArrayClass.hpp   */
/*                                                                            */
/* Primitive Array Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ArrayClass
#define Included_ArrayClass

#include "NumberArray.hpp"
#include "Numerics.hpp"

/**
 * The implementation of the Rexx Array class.
 */
class ArrayClass : public RexxObject
{
 public:

    enum
    {
        RaiseBoundsInvalid = 0x00000001,
        RaiseBoundsTooMany = 0x00000002,
        ExtendUpper        = 0x00000004,
        IndexAccess        = RaiseBoundsTooMany,
        IndexUpdate        = IndexAccess | RaiseBoundsInvalid | ExtendUpper,
    } IndexFlags;


    /**
     * Array internal class for performing recursive copies
     * for multi-dimensional arrays
     */
    class ElementCopier
    {
     public:

       void copy();
       void copyElements(size_t newDimension, size_t oldOffset, size_t newOffset);
       void copyBlocks(size_t dimension, size_t oldOffset, size_t newOffset);
       void getBlockSizes(size_t dimension, size_t &oldBlock, size_t &newBlock);

       ArrayClass *newArray;             // the array we're copying into
       ArrayClass *oldArray;             // the array we're copying from
       size_t totalDimensions;           // number of dimension to process
    };


    /**
     * A partition bounds instance used for sorting.
     */
    class PartitionBounds
    {
     public:
         enum
         {
             SmallRange = 10   // the size where we revert to an insertion sort
         };

         PartitionBounds(size_t l, size_t r) : left(l), right(r) {}
         PartitionBounds() : left(0), right(0) {}

         inline bool isSmall() { return (right - left) <= SmallRange; }
         inline size_t midPoint() { return (left + right) / 2; }

         size_t left;       // start of the range
         size_t right;
     };


    /**
     * Our base sort comparator, which just uses a compareTo
     * method.
     */
    class BaseSortComparator
    {
     public:
        inline BaseSortComparator() { }

        virtual wholenumber_t compare(RexxInternalObject *first, RexxInternalObject *second);
    };


    /**
     * Sorting comparator that uses a comparator object.
     */
    class WithSortComparator : public BaseSortComparator
    {
    public:
        inline WithSortComparator(RexxObject *c) : comparator(c) { }
        wholenumber_t compare(RexxInternalObject *first, RexxInternalObject *second) override;
    protected:
        RexxObject *comparator;
    };

    void * operator new(size_t, size_t = DefaultArraySize, size_t = DefaultArraySize);
    inline void operator delete(void *) {;}

    static ArrayClass *allocateNewObject(size_t size, size_t items, size_t maxSize, size_t type);


    inline ArrayClass(RESTORETYPE restoreType) { ; };
    inline ArrayClass() { ; };
    inline ArrayClass(RexxInternalObject *o1) { put(o1, 1); }
    inline ArrayClass(RexxInternalObject *o1, RexxInternalObject *o2) { put(o1, 1); put(o2, 2); }
    inline ArrayClass(RexxInternalObject *o1, RexxInternalObject *o2, RexxInternalObject *o3) { put(o1, 1); put(o2, 2); put(o3, 3); }
    inline ArrayClass(RexxInternalObject *o1, RexxInternalObject *o2, RexxInternalObject *o3, RexxInternalObject *o4) { put(o1, 1); put(o2, 2); put(o3, 3); put(o4, 4); }
           ArrayClass(RexxInternalObject **o, size_t c);
    inline ArrayClass(NumberArray *d) { dimensions = d; }

    inline ~ArrayClass() { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxInternalObject *copy() override;
    ArrayClass *makeArray() override;
    RexxString *primitiveMakeString() override;
    RexxString *makeString() override;

    ArrayClass   *allItems();
    ArrayClass   *allIndexes();
    RexxString   *toString(RexxString *, RexxString *);
    RexxInternalObject  *getRexx(RexxObject **, size_t);
    RexxInternalObject  *safeGet(size_t pos);
    void          put(RexxInternalObject * eref, size_t pos);
    RexxObject   *putRexx(RexxObject **, size_t);
    void          putApi(RexxInternalObject * eref, size_t pos);
    // this is virtual because Queue redefines this as a delete operation.
    virtual RexxInternalObject *remove(size_t);
    RexxInternalObject  *removeRexx(RexxObject **, size_t);
    RexxInternalObject  *removeItem(RexxInternalObject *target);
    RexxInternalObject  *removeItemRexx(RexxObject *target);
    RexxObject   *appendRexx(RexxObject *);
    size_t        append(RexxInternalObject *);
    void          appendAll(ArrayClass *);
    void          setExpansion(RexxObject * expansion);
    RexxInteger  *available(size_t position);
    // virtual so subclasses can screen out multidimensional support.
    virtual bool  validateIndex(RexxObject **, size_t, size_t, size_t, size_t &);
    inline bool   validateIndex(RexxObject *i, size_t start, size_t flags, size_t &p) { return validateIndex(&i, 1, start, flags, p); }
    // this is a nop for the Array class.
    virtual void  checkInsertIndex(size_t position) { }
    bool validateSingleDimensionIndex(RexxObject **index, size_t indexCount, size_t argPosition, size_t boundsError, size_t &position);
    bool validateMultiDimensionIndex(RexxObject **index, size_t indexCount, size_t argPosition, size_t boundsError, size_t &position);
    ArrayClass   *allocateArrayOfClass(size_t size);

    RexxInteger  *sizeRexx();
    RexxObject   *firstRexx();
    RexxObject   *lastRexx();
    RexxInternalObject *getFirstItem();
    RexxInternalObject *getLastItem();
    size_t       lastIndex();
    size_t       firstIndex();
    size_t       nextIndex(size_t index);
    RexxObject  *nextRexx(RexxObject **, size_t);
    size_t       previousIndex(size_t index);
    RexxObject  *previousRexx(RexxObject **, size_t);
    ArrayClass  *section(size_t, size_t);
    ArrayClass  *sectionRexx(RexxObject *, RexxObject *);
    RexxObject  *hasIndexRexx(RexxObject **, size_t);
    inline size_t items() { return itemCount; }
    RexxObject  *itemsRexx();
    RexxObject  *dimensionRexx(RexxObject *);
    ArrayClass  *getDimensionsRexx();
    size_t       getDimensions() { return isSingleDimensional() ? 1 : dimensions->size(); }
    size_t       dimensionSize(size_t i) { return dimensions == OREF_NULL || i > dimensions->size() ? 0 : dimensions->get(i); }
    SupplierClass *supplier();
    RexxObject  *join(ArrayClass *);
    void         extend(size_t);
    static size_t validateSize(RexxObject *size, size_t position);
    size_t      indexOf(RexxInternalObject *);
    RexxObject *indexRexx(RexxObject *target);
    void        extendMulti(RexxObject **, size_t, size_t);
    void        resize();
    inline void ensureSpace(size_t newSize)
    {
        // out of bounds?
        if (newSize > size())
        {
            // expand to at least the given size
            extend(newSize);
        }
    }
    inline bool isFixedDimension() { return dimensions != OREF_NULL || size() != 0; }

    RexxObject  *newRexx(RexxObject **, size_t);
    RexxObject  *ofRexx(RexxObject **, size_t);
    RexxObject  *empty();
    bool         isEmpty();
    RexxObject  *isEmptyRexx();
    RexxObject  *fillRexx(RexxObject *);
    void         fill(RexxInternalObject *);
    RexxObject  *hasItemRexx(RexxObject *);
    bool         hasItem(RexxInternalObject *target);
    bool         hasIdentityItem(RexxInternalObject *target);
    wholenumber_t sortCompare(RexxObject *comparator, RexxInternalObject *left, RexxInternalObject *right);
    ArrayClass  *stableSortRexx();
    ArrayClass  *stableSortWithRexx(RexxObject *comparator);
    RexxObject  *insertRexx(RexxObject *value, RexxObject *index);
    size_t       insert(RexxInternalObject *value, size_t index);
    RexxInternalObject  *deleteRexx(RexxObject *index);
    RexxInternalObject  *deleteItem(size_t index);

    inline size_t       addLast(RexxInternalObject *item) { return append(item); }
    inline size_t       addFirst(RexxInternalObject *item) { return insert(item, 1); }
    inline RexxInternalObject *removeLast() { return remove(lastIndex()); }
    inline RexxInternalObject *removeFirst() { return deleteItem(firstIndex()); }
    inline size_t       insertAfter(RexxInternalObject *item, size_t index) { return insert(item, index); }
    inline ArrayClass  *array() { return makeArray(); }
    inline size_t       size() { return expansionArray->arraySize; }
    inline bool         isOccupied(size_t pos) { return get(pos) != OREF_NULL; }
    inline bool         isInbounds(size_t pos) { return pos > 0 && pos <= size(); }
    inline bool         hasIndex(size_t pos) { return isInbounds(pos) && isOccupied(pos); }
           void         updateLastItem();
           void         setArrayItem(size_t position, RexxInternalObject *value);
           void         clearArrayItem(size_t position);
           void         copyArrayItem(size_t position, RexxInternalObject *value);
           void         setOrClearArrayItem(size_t position, RexxInternalObject *value);
    inline void         zeroItem(size_t position) { data()[position - 1] = OREF_NULL; }
           void         clearItem(size_t position);
           // NOTE:  only to be used during sorting!
    inline void         setSortItem(size_t position, RexxInternalObject *value) { expansionArray->objects[position - 1] = value; }
           void         setItem(size_t position, RexxInternalObject *value);
           void         checkMultiDimensional(const char *methodName);
           void         shrink(size_t amount);

    // check if we need to update the itemcount when writing to a given position.
    inline void checkSetItemCount(size_t pos)
    {
        if (!isOccupied(pos))
        {
            itemCount++;
        }
    }
    // check if clearing an index position affects the last item position
    inline void checkClearLastItem(size_t pos)
    {
        if (pos == lastItem)
        {
            updateLastItem();
        }
    }

    // check if we need to update the itemcount when writing a given position.
    inline void checkClearItemCount(size_t pos)
    {
        if (isOccupied(pos))
        {
            itemCount--;
        }
    }

    // check if we need to update the lastItem position after adding an object to a position
    inline void checkLastItem(size_t pos)
    {
        if (pos > lastItem)
        {
            lastItem = pos;
        }
    }

    inline RexxInternalObject  *get(size_t pos) { return (data())[pos-1];}
    inline RexxInternalObject **data() { return expansionArray->objects; }
    inline RexxInternalObject **data(size_t pos) { return &((data())[pos-1]);}
    inline RexxObject **messageArgs() { return (RexxObject **)data(); }
    inline size_t       messageArgCount() { return lastItem; }

    inline ArrayClass   *getExpansion() { return expansionArray; }
    size_t              findSingleIndexItem(RexxInternalObject *item);
    RexxObject *        indexToArray(size_t idx);
    RexxObject *        convertIndex(size_t idx);

    inline bool isMultiDimensional() { return dimensions != OREF_NULL && dimensions->size() != 1; }
    inline bool isSingleDimensional() { return !isMultiDimensional(); }
    inline bool hasExpanded() { return expansionArray != this && expansionArray != OREF_NULL; }

    static ArrayClass *createMultidimensional(RexxObject **dims, size_t count, RexxClass *);
    static inline ArrayClass *createMultidimensional(ArrayClass *dims, RexxClass *c)
    {
        return createMultidimensional((RexxObject **)dims->data(), dims->items(), c);
    }

    static void createInstance();
    // singleton class instance;
    static RexxClass *classInstance;
    static ArrayClass *nullArray;

    static const size_t DefaultArraySize = 16;     // default size for ooRexx allocation
    // maximum Array size we can handle
    static const size_t MaxFixedArraySize = (Numerics::MAX_WHOLENUMBER / 10) + 1;

 protected:

    void         mergeSort(BaseSortComparator &comparator, ArrayClass *working, size_t left, size_t right);
    void         merge(BaseSortComparator &comparator, ArrayClass *working, size_t left, size_t mid, size_t right);
    static void  arraycopy(ArrayClass *source, size_t start, ArrayClass *target, size_t index, size_t count);
    size_t       find(BaseSortComparator &comparator, RexxInternalObject *val, int bnd, size_t left, size_t right);
    void         openGap(size_t index, size_t elements);
    void         closeGap(size_t index, size_t elements);
    inline RexxInternalObject **slotAddress(size_t index) { return &(data()[index - 1]); }
    inline size_t       dataSize() { return ((char *)slotAddress(size() + 1)) - ((char *)data()); }

    static const size_t MinimumArraySize;      // the minimum size we allocate.
    // for small Arrays, we expand by doubling the current size, however
    // for Arrays larger than this limit, we just extend by half the current size
    static const size_t ExpansionDoubleLimit = 2000;

    size_t arraySize;                   // current logical size of the array
    size_t maximumSize;                 // The allocation size of the array
    size_t lastItem;                    // location of last set element
    size_t itemCount;                   // the count of items in the array
    NumberArray *dimensions;            // Array containing dimensions - null if 1-dimensional
    ArrayClass *expansionArray;         // actual array containing data (will be self-referential originall)
    RexxInternalObject *objects[1];     // the start of the array of stored objects.
};


/**
 * Make a zero-length array item.
 *
 * @return A new array.
 */
inline ArrayClass *new_array()
{
    return new ((size_t)0) ArrayClass;
}


/**
 * Create an array with a given size.
 *
 * @param s      The size of the array.
 *
 * @return The new array item.
 */
inline ArrayClass *new_array(size_t s)
{
    return new (s) ArrayClass;
}


/**
 * Create an array item with a given set of dimensions.
 *
 * @param s      The size of the array.
 * @param dims   The dimensions for this array.
 *
 * @return A new array item.
 */
inline ArrayClass *new_array(size_t s, NumberArray *dims)
{
    return new (s) ArrayClass(dims);
}


/**
 * Create an array populated with objects from another source.
 *
 * @param s      The number of objects.
 * @param o      The pointer to the set of objects.
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(size_t s, RexxInternalObject **o)
{
    return new (s) ArrayClass(o, s);
}


/**
 * Create an array populated with objects from another source.
 *
 * @param s      The number of objects.
 * @param o      The pointer to the set of objects.
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(size_t s, RexxObject **o)
{
    return new (s) ArrayClass((RexxInternalObject **)o, s);
}


/**
 * Create a new array with one item.
 *
 * @param o1     The object to add to the array.
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(RexxInternalObject *o1)
{
    return new (1) ArrayClass(o1);
}


/**
 * Create a new array with two items.
 *
 * @param o1     The first object to add to the array.
 * @param o2     The second object to add
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(RexxInternalObject *o1, RexxInternalObject *o2)
{
    return new (2) ArrayClass(o1, o2);
}


/**
 * Create a new array with three items.
 *
 * @param o1     The first object to add to the array.
 * @param o2     The second object to add
 * @param o3     The third object to add.
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(RexxInternalObject *o1, RexxInternalObject *o2, RexxInternalObject *o3)
{
    return new (3) ArrayClass(o1, o2, o3);
}


/**
 * Create a new array with four items.
 *
 * @param o1     The first object to add to the array.
 * @param o2     The second object to add
 * @param o3     The third object to add.
 * @param o4     The fourth object to add.
 *
 * @return A new array object.
 */
inline ArrayClass *new_array(RexxInternalObject *o1, RexxInternalObject *o2, RexxInternalObject *o3, RexxInternalObject *o4)
{
    return new (4) ArrayClass(o1, o2, o3, o4);
}

 #endif

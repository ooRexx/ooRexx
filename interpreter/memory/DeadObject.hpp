/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                              DeadObject.hpp    */
/*                                                                            */
/* Primitive DeadObject class definitions                                     */
/*                                                                            */
/******************************************************************************/

#ifndef Included_DeadObject
#define Included_DeadObject

void FOUND(); void NOTFOUND();
/* Dead chains are doubly linked lists.  The anchors are in the memoryobj dead   */
/* arrays.  The first element of each dead list is a dead object 3 words long.   */
/* The first and third words are next and prev pointers, and the second is the   */
/* header, just like any object.  However, the header is set to 0 so that this   */
/* element will never be used and we can recognize if the chain has useable      */
/* elements with the expression:  element->next->header.                         */


/* DeadObject must be kept in synch with RexxInternalObject size */
/* and layout */
class DeadObject {
 friend class DeadObjectPool;

 public:
    inline void *operator new(size_t size, void *address) { return address; };
    inline void  operator delete(void *, void *) {;}

    inline void addEyeCatcher(const char *string) { memcpy(VFT, string, 4); }
    inline DeadObject(size_t objectSize) {
        header.setObjectSize(objectSize);
#ifdef CHECKOREFS
        addEyeCatcher("DEAD");
#endif
    }

  // Following is a static constructor.
  // Called during RexxMemory initialization
  inline DeadObject() {
      addEyeCatcher("HEAD");
      header.setObjectSize(0);
      /* Chain this deadobject to itself. */
      next = this;
      previous = this;
  }

  inline void setObjectSize(size_t newSize) { header.setObjectSize(newSize); }
  inline size_t getObjectSize() {  return header.getObjectSize(); };

  inline void insertAfter(DeadObject *newDead) {
      newDead->next     = this->next;
      newDead->previous = this;
      this->next->previous = newDead;
      this->next           = newDead;
  };

  inline void insertBefore(DeadObject *newDead) {
      newDead->next        = this;
      newDead->previous    = this->previous;
      this->previous->next = newDead;
      this->previous       = newDead;
  };

  inline void remove() {
      this->next->previous = this->previous;
      this->previous->next = this->next;
  }

  inline bool isReal() { return header.getObjectSize() != 0; }
  inline bool isHeader() { return header.getObjectSize() == 0; }

  inline void reset() {
      /* Chain this deadobject to itself, removing all of the */
      /* elements from the chain */
      next = this;
      previous = this;
  }

  inline DeadObject *end() { return (DeadObject *)(((char *)this) + this->getObjectSize()); }
  inline bool overlaps(DeadObject *o) { return (o >= this && o < end()) || (o->end() >= this && o->end() < this->end()); }


protected:
  char        VFT[sizeof(void *)];     /* Place Holder for virtualFuncTable */
                                       /* in debug mode, holds string DEAD  */
  ObjectHeader  header;                /* header info just like any obj     */
  DeadObject *next;                    /* next dead object on this chain    */
  DeadObject *previous;                /* prev dead object on this chain    */
};

/* A pool of dead objects.  This is the anchor for a set of dead */
/* objects, as well as control and metric information for what is */
/* stored in the pool of objects. */
class DeadObjectPool
{
  public:
    inline DeadObjectPool() { init("Generic DeadChain"); }

    inline DeadObjectPool(const char * poolID) : anchor() {
        init(poolID);
    }

    inline void init(const char * poolID) {
        this->id = poolID;
#ifdef MEMPROFILE                      /* doing memory profiling            */
        allocationCount = 0;
        allocationReclaim = 0;
        allocationHits = 0 ;
        allocationMisses = 0;
#endif
    }

    // the threshold for deciding the large object chain is getting fragmented.
    // this tells us we need to move larger blocks to the front of the chain.
    #define ReorderThreshold 100

    inline void  setID(const char *poolID) { this->id = poolID; }
    inline void  empty() { anchor.reset(); }
    inline bool  isEmpty() { return anchor.next->isReal(); }
    inline void  emptySingle() { anchor.next = NULL; }
    inline bool  isEmptySingle() { return anchor.next == NULL; }
    inline
           void  checkObjectGrain(DeadObject *obj);
    inline void  add(DeadObject *obj) {
//      checkObjectOverlap(obj);
//      checkObjectGrain(obj);
        anchor.insertAfter(obj);
    }
    void addSortedBySize(DeadObject *obj);
    void addSortedByLocation(DeadObject *obj);
    void dumpMemoryProfile(FILE *outfile);
    void checkObjectOverlap(DeadObject *obj);

    inline DeadObject *getFirst()
    /******************************************************************************/
    /* Function:  Get the first object from the deal object pool.  If the pool    */
    /* is empty, this returns NULL.  If a block is returned it is removed from the*/
    /* pool before return.                                                        */
    /******************************************************************************/
    {
        DeadObject *newObject = anchor.next;
        /* The next item could just be a pointer back to the anchor. */
        /* If it is not, we have a real block to return. */
        if (newObject->isReal()) {
            /* we need to remove the object from the chain before */
            /* returning it. */
            newObject->remove();
            logHit();
            return newObject;
        }
        logMiss();
        return NULL;
    }
    inline DeadObject *lastBlock() { return anchor.previous; }
    inline DeadObject *firstBlock() { return anchor.next; }
    inline DeadObject *findFit(size_t length)
    /******************************************************************************/
    /* Function:  Find first object large enough to satisfy this request.  If the */
    /* pool is empty, this returns NULL.  If a block is returned it is removed    */
    /* from the pool before return.                                               */
    /******************************************************************************/
    {
        DeadObject *newObject = anchor.next;
        size_t newLength;
        for (newLength = newObject->getObjectSize(); newLength != 0; newLength = newObject->getObjectSize()) {
            if (newLength >= length) {
                newObject->remove();
                logHit();
                return newObject;
            }
            newObject = newObject->next;
        }
        logMiss();
        return NULL;
    }

    inline DeadObject *findFit(size_t length, size_t *realLength)
    /******************************************************************************/
    /* Function:  Find first object large enough to satisfy this request.  If the */
    /* pool is empty, this returns NULL.  If a block is returned it is removed    */
    /* from the pool before return.                                               */
    /******************************************************************************/
    {
        DeadObject *newObject = anchor.next;
        size_t newLength;
        int probes = 1;
        for (newLength = newObject->getObjectSize(); newLength != 0; newLength = newObject->getObjectSize()) {
            if (newLength >= length) {
                // we had to examine a lot of objects to get a match.
                // it's worthwhile percolating the larger objects on the rest of the
                // chain toward the front.  We only do this when we're starting to have problems
                // allocating objects because of fragmentation.
                DeadObject *tailObject = newObject->next;

                newObject->remove();
                logHit();
                *realLength = newLength;
                if (probes > ReorderThreshold)
                {
                    for (size_t tailLength = tailObject->getObjectSize(); tailLength != 0; tailLength = tailObject->getObjectSize())
                    {
                        // the size we just had problems with is a good marker for
                        // selecting candidates to move toward the front.  The will guarantee
                        // that a similar request for the same size will succeed faster in the future.
                        DeadObject *nextObject = tailObject->next;
                        if (tailLength > length)
                        {
                            tailObject->remove();
                            add(tailObject);
                        }
                        tailObject = nextObject;
                    }
                }
                return newObject;
            }
            probes++;
            newObject = newObject->next;
        }
        logMiss();
        return NULL;
    }
    DeadObject *findBestFit(size_t length);
    DeadObject *findSmallestFit(size_t minSize);

    inline void  addSingle(DeadObject *obj) {
//      checkObjectOverlap(obj);
//      checkObjectGrain(obj);
        obj->next = anchor.next;
        anchor.next = obj;
    }


    inline DeadObject *getFirstSingle()
    /******************************************************************************/
    /* Function:  Get the first object from the deal object pool.  If the pool    */
    /* is empty, this returns NULL.  If a block is returned it is removed from the*/
    /* pool before return.                                                        */
    /******************************************************************************/
    {
        DeadObject *newObject = anchor.next;

        /* if we have a real object remove it */
        if (newObject != NULL) {
            logHit();
            anchor.next = newObject->next;
            return newObject;
        }
        logMiss();
        return NULL;
    }

  private:
#ifdef MEMPROFILE                      /* doing memory profiling            */
    inline void    logAllocation()  { allocationCount++; }
    inline void    logReclaim()  { allocationReclaim++; }
    inline void    logHit()  { allocationHits++; }
    inline void    logMiss() { allocationMisses; }
    inline void    clearProfile() { allocationCount = 0; allocationReclaim = 0; allocationHits = 0; allocationMisses = 0; }
#else
    inline void    logAllocation() { ; }
    inline void    logReclaim()  { ; }
    inline void    logHit()  { ; }
    inline void    logMiss()  { ; }
    inline void    clearProfile();
#endif

    /* the anchor position for our chain */
    DeadObject anchor;                 /* the anchor position for our chain */
    const char *id;                    /* identifier for the pool */
#ifdef MEMPROFILE                      /* doing memory profiling            */
    size_t     allocationCount;        /* the number of allocations from this chain */
    size_t     allocationReclaim;      /* elements we've split into multiples for reuse */
    size_t     allocationHits;         /* successful allocation requests */
    size_t     allocationMisses;       /* unsuccessful allocation requests */
#endif

};

#endif

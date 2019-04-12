/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX Kernel                                           Memorysegment.hpp    */
/*                                                                            */
/* Primitive MemorySegment class definitions                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Included_MemorySegment
#define Included_MemorySegment

#include "DeadObject.hpp"
#include "Interpreter.hpp"
#include "Memory.hpp"
#include <stdio.h>

class MemoryObject;

/* A segment of heap memory. A MemorySegment will be associated */
/* with a particular MemorySegmentSet, which implements the */
/* suballocation rules. */
class MemorySegmentHeader
{
 friend class MemorySegmentSet;
 friend class NormalSegmentSet;
 friend class LargeSegmentSet;
 friend class OldSegmentSet;
 friend class MemoryObject;

  protected:

   size_t segmentSize;                     /* size of the segment */
   size_t liveObjects;                     /* number of live objects in segment */
   MemorySegment *next;                    /* next segment in the chain */
   MemorySegment *previous;                /* previous segment in the chain */
};


/* A segment of heap memory. A MemorySegment will be associated */
/* with a particular MemorySegmentSet, which implements the */
/* suballocation rules. */
class MemorySegment : public MemorySegmentHeader
{
 friend class MemorySegmentSet;
 friend class NormalSegmentSet;
 friend class LargeSegmentSet;
 friend class SingleObjectSegmentSet;
 friend class OldSegmentSet;
 friend class MemoryObject;

 public:
   inline MemorySegment(size_t segSize)
   {
       segmentSize = segSize - sizeof(MemorySegmentHeader);
   }
   /* Following is a static constructor, called during MemoryObject */
   /* initialization */
   inline MemorySegment()
   {
       segmentSize = 0;
       /* Chain this segment to itself. */
       next = this;
       previous = this;
   }

   inline void insertAfter(MemorySegment *newSegment)
   {
       newSegment->next     = this->next;
       newSegment->previous = this;
       this->next->previous = newSegment;
       this->next           = newSegment;
   };

   inline void insertBefore(MemorySegment *newSegment)
   {
       newSegment->next     = this;
       newSegment->previous = this->previous;
       this->previous->next = newSegment;
       this->previous       = newSegment;
   };

   inline void remove()
   {
       this->next->previous = this->previous;
       this->previous->next = this->next;
   }

   inline void removeAll()
   {
       firstObject()->remove();
       remove();
   }
   void transferSegment(MemorySegment *segment);

   inline bool isInSegment(RexxInternalObject *object)
   {
       return (((char *)object >= segmentStart) && ((char *)object <= segmentStart + segmentSize));
   }

   inline DeadObject *createDeadObject() { return new ((void *)segmentStart) DeadObject(segmentSize); }

   inline DeadObject *firstObject() { return (DeadObject *)segmentStart; }
   inline size_t size() { return segmentSize; }
   inline size_t realSize() { return segmentSize + MemorySegmentOverhead; }
   inline char *start() { return segmentStart; }
   inline char *end() { return segmentStart + segmentSize; }
   inline RexxInternalObject *startObject() { return (RexxInternalObject *)start(); }
   inline RexxInternalObject *endObject() { return (RexxInternalObject *)end(); }

   inline bool isReal() { return segmentSize != 0; }
   inline bool isEmpty() { return liveObjects == 0; }
   void   dump(const char *owner, size_t counter, FILE *keyfile, FILE *dumpfile);
   void gatherObjectStats(MemoryStats *memStats, SegmentStats *stats);
   void markAllObjects();

   // This rounds to segment sized chunks, not taking the overhead into account.
   static inline size_t roundSegmentBoundary(size_t n) { return Memory::roundUp(n, SegmentSize); }

   static const size_t MemorySegmentOverhead = sizeof(MemorySegmentHeader);

   // default size for a segment allocation, we go larger on 64-bit
   static const size_t SegmentSize = (256 * Memory::LargeAllocationUnit * 2);
   // our threshold for moving to a larger block allocation scheme
   static const size_t LargeBlockThreshold = Memory::LargeAllocationUnit * 4;
   // our threshold for moving to an object per block allocation scheme
   // a default segment size seems a logical threshold
   static const size_t SingleBlockThreshold = SegmentSize;

   char segmentStart[sizeof(DeadObject)];  // start of the object data
};


/* A set of memory segments.  This manages the access to a pool of */
/* memory segments allocated for different uses by MemoryObject. */
/* This is a subclass of MemorySegment because the MemorySegmentSet */
/* object is also the anchor element for the segment chaining. */
class MemorySegmentSet
{
    friend class MemoryObject;

  public:
      typedef enum { SET_UNINITIALIZED, SET_NORMAL, SET_LARGEBLOCK, SET_OLDSPACE, SET_SINGLEOBJECT } SegmentSetID;
        /* the memory segment mimic for anchoring the pool */
      MemorySegmentSet(MemoryObject *memObject, SegmentSetID id, const char *setName)
      {
          /* Chain this segment to itself.     */
          owner = id;
          count = 0;
          /* keep the link back to the memory object that provides */
          /* us services. */
          this->memory = memObject;
          this->name = setName;
      }
        /* the default constructor */
      MemorySegmentSet()
      {
          /* Chain this segment to itself.     */
          owner = SET_UNINITIALIZED;
          count = 0;
          /* The link to the memory object will need to be established later */
          memory = NULL;
      }

      virtual ~MemorySegmentSet() { ; }

      /* Following is a static constructor, called during */
      /* MemoryObject initialization */

      inline void removeSegment(MemorySegment *segment)
      {
          /* remove both the segment, and any blocks on the dead */
          /* chains. */
          segment->remove();
          count--;
      }

      inline void removeSegmentAndStorage(MemorySegment *segment)
      {
          /* remove both the segment, and any blocks on the dead */
          /* chains. */
          segment->removeAll();
          count--;
      }

      inline void add(MemorySegment *segment)
      {
          anchor.insertBefore(segment);
          count++;
      }

      inline MemorySegment *first()
      {
          if (anchor.next->isReal())
          {
              return anchor.next;
          }
          else
          {
              return NULL;
          }
      }

      inline MemorySegment *next(MemorySegment *segment)
      {
          if (segment->next->isReal())
          {
              return segment->next;
          }
          else
          {
              return NULL;
          }
      }

      void transferSegment(MemorySegment *segment);

      void dumpSegments(FILE *keyfile, FILE *dumpfile);
      void addSegment(MemorySegment *segment);
      void sweep();
      void sweepSingleSegment(MemorySegment *sweepSegment);
      inline bool is(SegmentSetID id) { return owner == id; }
      void gatherStats(MemoryStats *memStats, SegmentStats *stats);
      MemorySegment *largestActiveSegment();

      virtual void   dumpMemoryProfile(FILE *outfile);
      virtual DeadObject *donateObject(size_t allocationLength);

      static const size_t MinimumSegmentSize;
      // amount of usable space in a minimum sized segment
      static const size_t MinimumSegmentDeadSpace;
      // default size for a larger segment allocation
      static const size_t LargeSegmentSize;
      // allocation available in a default segment
      static const size_t SegmentDeadSpace;
      // space available in a larger allocation.
      static const size_t LargeSegmentDeadSpace;

  protected:

      virtual void addDeadObject(DeadObject *object);
      virtual void addDeadObject(char *object, size_t length);
      RexxInternalObject *splitDeadObject(DeadObject *object, size_t allocationLength, size_t splitMinimum);
      void insertSegment(MemorySegment *segment);
      virtual size_t suggestMemoryExpansion();
      virtual void prepareForSweep();
      virtual void completeSweepOperation();
      void adjustMemorySize();
      bool newSegment(size_t requestLength, size_t minimumLength);

      virtual MemorySegment *allocateSegment(size_t requestLength, size_t minimumLength);
      inline float freeMemoryPercentage() {return (float)deadObjectBytes/(float)(deadObjectBytes + liveObjectBytes);  }
      inline size_t totalFreeMemory() { return liveObjectBytes + deadObjectBytes; }
      /* This rounds a size into an even segment multiple, taking the */
      /* segment overhead into account. */
      inline size_t calculateSegmentAllocation(size_t n)
      {
          size_t size = MemorySegment::roundSegmentBoundary(n) - MemorySegment::MemorySegmentOverhead;
          /* this could be true if our size is larger than can fit into a */
          /* segment once the overhead is considered.  If we can't fit, */
          /* we go over by a segment. */
          if (size < n)  {
              size += MemorySegment::SegmentSize;
          }
          return size;
      }
      void addSegments(size_t requiredSpace);

      inline void validateObject(size_t bytes)
      {
      #ifdef CHECKOREFS
          /* is object invalid size?           */
          if (!MemoryObject::isValidSize(bytes)) {
              /* Yes, this is not good.  Exit      */
              /* Critical Section and report       */
              /* unrecoverable error.              */
              Interpreter::logicError("Bad object detected during Garbage Collection, unable to continue");
          }
      #endif
      }



    MemorySegment anchor;                 /* the anchor for our active segment chain */
    size_t  count;                        /* the number of elements in the pool */
    size_t  liveObjectBytes;              /* bytes allocation to live objects */
    size_t  deadObjectBytes;              /* bytes consumed by dead objects */
    SegmentSetID owner;                   /* the owner of this segment */
    const char  *name;                    /* character identifier for debugging/profiling */
    MemoryObject *memory;                 /* the hosting memory object */
};


/**
 * The segment set used for "normal" allocations.  This
 * segment set will be used for smaller allocations, particularly
 * ones that can be allocated from one of the small allocation pools.
 */
class NormalSegmentSet : public MemorySegmentSet
{
  public:

    /* the default constructor */
    NormalSegmentSet()  { ; }
    NormalSegmentSet(MemoryObject *memory);
    virtual ~NormalSegmentSet() { ; }

    void   dumpMemoryProfile(FILE *outfile) override;
    inline RexxInternalObject *allocateObject(size_t allocationLength)
    {
        DeadObject *newObject;
        size_t targetPool;
        size_t realLength;

        /* Calculate the dead chain.  Note that if this is larger than */
        /* the largest subpool, the for() loop test below will fail, */
        /* causing this to drop down to the large block allocation */
        /* logic.  This eliminates an additional check against the */
        /* large size. */
        targetPool = lengthToDeadPool(allocationLength);

        if (targetPool < DeadPools)
        {

            /* pick up the last successful one */
            size_t currentDead = lastUsedSubpool[targetPool];
            /* loop through the small pool chains looking for a block. */
            /* We only go up to the largest blocks as a last resort to */
            /* reduce the fragmentation. */
            while (currentDead < DeadPools)
            {
                /* See if the chain has an object.  Once we get an */
                /* object, we return this directly.  We accept over */
                /* allocations when then come from the subpool chain. */
                /* Since this is such a heavily hit path, we don't want */
                /* to absorb the overhead of attempting to split the */
                /* blocks.  For the majority of over allocations, we */
                /* can't split anyway.  When we do split, the result is */
                /* a very small fragment. */
                newObject = subpools[currentDead].getFirstSingle();
                if (newObject != OREF_NULL)
                {
                    /* Record the success.  Next time around, */
                    /* allocations will come directly here. */
                    lastUsedSubpool[targetPool] = currentDead;
                    /* we have a block.  Now see if we got this from a */
                    /* higher level chain and have enough room to */
                    /* subdivide into a small block. */
                    /* Convert this from a dead object into a real one of the */
                    /* given size. */
                    return (RexxInternalObject *)newObject;
                }

                currentDead++;

                while (currentDead < DeadPools)
                {
                    if (lastUsedSubpool[currentDead] < DeadPools)
                    {
                        // this pool might be redirected already, so
                        // pick up the index of where it's redirected to.
                        currentDead = lastUsedSubpool[currentDead];
                        lastUsedSubpool[targetPool] = currentDead;
                        break;
                    }
                    currentDead++;
                }
            }
            /* we've gone all the way to the end without finding */
            /* anything.  Cause the next allocation to skip directly to */
            /* the large chain. */
            lastUsedSubpool[targetPool] = DeadPools;
        }
        /* Nope, go through the LARGEDEAD object looking for the 1st */
        /* one we can use either our object is too big for all the */
        /* small chains, or the small chains are depleted.... */
        /* Go through the LARGEDEAD object looking for the 1st */
        /* one we can use either our object is too big for all the */
        /* small chains, or the small chains are depleted.... */
        newObject = largeDead.findFit(allocationLength, &realLength);
        if (newObject != NULL)
        {         /* did we find an object?            */
            size_t deadLength = realLength - allocationLength;
            /* remainder too small or this is a very large request */
            /* is the remainder two small to reuse? */
            if (deadLength < Memory::MinimumObjectSize)
            {
                /* Convert this from a dead object into a real one of the */
                /* given size. */
                return (RexxInternalObject *)newObject;
            }
            else
            {
                /* potentially split this object into a smaller unit so we */
                /* can reuse the remainder. */
                return splitNormalDeadObject(newObject, allocationLength, deadLength);
            }
        }
        return OREF_NULL;
    }

    RexxInternalObject *handleAllocationFailure(size_t allocationLength);
    DeadObject *donateObject(size_t allocationLength) override;
    void    getInitialSet();
    size_t suggestMemoryExpansion() override;

  protected:
    void addDeadObject(DeadObject *object) override;
    void addDeadObject(char *object, size_t length) override;
    void prepareForSweep() override;
    void completeSweepOperation() override;

  private:

    // index of first dead free chain.  We start with the previous
    // chain, as older tokenized images can contain objects smaller
    // than our minimum.  If these are garbage collected individually,
    // we need a place to put them.
    static const size_t FirstDeadPool = Memory::MinimumObjectSize / Memory::ObjectGrain;
    // The largest size element we'll keep in a subpool
    static const size_t LargestSubpool = 512;
    // The index of the last subpool dead chain
    static const size_t LastDeadPool = LargestSubpool / Memory::ObjectGrain;
    // number of free chains (we index zero based, so we need to
    // allocate one additional pool)
    static const size_t DeadPools = LastDeadPool + 1;

    // the threshold to trigger expansion of the normal segment set.
    static const double NormalMemoryExpansionThreshold;
    // allocation request for the recovery segment
    static const size_t RecoverSegmentSize = ((MemorySegment::SegmentSize/2) - MemorySegment::MemorySegmentOverhead);
    // initial allocation size for normal space.
    static const size_t InitialNormalSegmentSpace;

    // map an object length to an allocation deadpool.  NOTE:  this
    // assumes the length has already been rounded to ObjectGrain!
    static inline size_t lengthToDeadPool(size_t l) { return ((l) / Memory::ObjectGrain); }
    // map a dead pool index to the size of blocks held in the pool
    static inline size_t deadPoolToLength(size_t d) { return ((d) * Memory::ObjectGrain); }

    inline size_t mapLengthToDeadPool(size_t length) { return length / Memory::ObjectGrain; }
    RexxInternalObject *findLargeDeadObject(size_t allocationLength);
    inline size_t recommendedMemorySize() { return (size_t)((float)liveObjectBytes/(1.0 - NormalMemoryExpansionThreshold)); }
    void checkObjectOverlap(DeadObject *obj);
    RexxInternalObject *findObject(size_t allocationLength);
    inline RexxInternalObject *splitNormalDeadObject(DeadObject *object, size_t allocationLength, size_t deadLength)
    {
        /* we need to keep all of these sizes as ObjectGrain multiples, */
        /* so round it down...the allocation will get all of the extra. */
        /* deadLength = rounddown(deadLength, ObjectGrain);
           allocationLength is rounded, deadLength might be
           an ungrained object size from old tokenized format */

        /* Yes, so pull new object out of the front of the dead */
        /* object, adjust the size of the Dead object.  We want */
        /* to use the front rather than the back so that if we */
        /* hit the need to split a segment because of low memory */
        /* conditions, we increase the probability that we'll be */
        /* able to use the end of the segment. */
        DeadObject *largeObject = (DeadObject *)(((char *)object) + allocationLength);
        /* if the length is larger than the biggest subpool we */
        /* maintain, we add this to the large block list. */
        if (deadLength > LargestSubpool)
        {
              /* ideally, we'd like to add this sorted by size, but */
              /* this is called so frequently, attempting to sort */
              /* degrades performance by about 10%. */
             largeDead.add(new (largeObject) DeadObject(deadLength));
        }
        else
        {
            /* calculate the dead chain          */
            /* and add that to the appropriate chain */
            size_t deadChain = lengthToDeadPool(deadLength);
            subpools[deadChain].addSingle(new (largeObject) DeadObject(deadLength));
            /* we can mark this subpool as having items again */
            lastUsedSubpool[deadChain] = deadChain;
        }
        /* Convert this from a dead object into a real one of the */
        /* given size. */
        ((RexxInternalObject *)object)->setObjectSize(allocationLength);
        return (RexxInternalObject *)object;
    }

    DeadObjectPool largeDead;             /* the set of large dead objects */
    DeadObjectPool subpools[DeadPools];   /* our set of allocation subpools */
    size_t lastUsedSubpool[DeadPools + 1];/* a look-aside index to tell us what pool to use for a given size */
    MemorySegment *recoverSegment;        /* our last-ditch memory segment */
};


/**
 * A segment set used for allocating "larger" objects, but not necessarily
 * the largest objects.
 */
class LargeSegmentSet : public MemorySegmentSet
{
  public:

    /* the default constructor */
    LargeSegmentSet()  { ; }
    LargeSegmentSet(MemoryObject *memory);
    virtual ~LargeSegmentSet() { ; }

    void   dumpMemoryProfile(FILE *outfile) override;
    RexxInternalObject *handleAllocationFailure(size_t allocationLength);
    inline RexxInternalObject *allocateObject(size_t allocationLength)
    {
        DeadObject *largeObject;

        /* go through the LARGEDEAD object looking for the 1st one we can */
        /* use either our object is too big for all the small chains, or */
        /* the small chain are depleted.... */
        largeObject = deadCache.findBestFit(allocationLength);
        /* did we find an object?            */
        if (largeObject != NULL)
        {
            /* remember the successful request */
            requests++;
            /* split and prepare this object for use */
            return splitDeadObject(largeObject, allocationLength, Memory::LargeAllocationUnit);
        }
        return OREF_NULL;                    /* we couldn't get this              */
    }

    DeadObject *donateObject(size_t allocationLength) override;
    void    getInitialSet();

protected:

    // initial allocation size for large space.
    static const size_t InitialLargeSegmentSpace;

    void addDeadObject(DeadObject *object) override;
    void addDeadObject(char *object, size_t length) override;
    MemorySegment *allocateSegment(size_t requestLength, size_t minimumLength) override;
    size_t suggestMemoryExpansion() override;
    void expandSegmentSet(size_t allocationLength);
    void prepareForSweep() override;
    void completeSweepOperation() override;

  private:

    // the threshold to trigger expansion of the normal segment set.
    static const double LargeMemoryExpansionThreshold;
    inline size_t recommendedMemorySize() { return (size_t)((float)liveObjectBytes/(1.0 - LargeMemoryExpansionThreshold)); }

    RexxInternalObject *findObject(size_t allocationLength);

    DeadObjectPool deadCache;             /* the set of large dead objects */
    size_t         requests;              /* requests since last gc cycle. */
    size_t         smallestObject;        // the smallest object in the set
    size_t         largestObject;         // the largest object in the set
};


/**
 * A segment set used for allocating the largest objects. We
 * allocate a segment dedicated to each object and will return
 * the segment when the object is garbage collected.
 */
class SingleObjectSegmentSet : public MemorySegmentSet
{
  public:

    /* the default constructor */
    SingleObjectSegmentSet()  { ; }
    SingleObjectSegmentSet(MemoryObject *memory);
    virtual ~SingleObjectSegmentSet() { ; }

    void   dumpMemoryProfile(FILE *outfile) override;
    RexxInternalObject *handleAllocationFailure(size_t allocationLength);
    RexxInternalObject *allocateObject(size_t allocationLength);

protected:

    void addDeadObject(DeadObject *object) override;
    void addDeadObject(char *object, size_t length) override;
    MemorySegment *allocateSegment(size_t requestLength, size_t minimumLength) override;
    void completeSweepOperation() override;

  private:

    // the limit of new segments we will allocate before forcing a GC.
    // this is kept fairly small, since repeated allocations is frequently a
    // sign that we have segments we can release.
    static const size_t ForceGCThreshold = 4;

    size_t         allocationsSinceLastGC;// number of objects allocated since the last GC.
    size_t         allocationCount;       // total number of allocations
    size_t         returnCount;           // the count of segments we've returned to system
};


/**
 * The segment set for the oldspace containing the ooRexx image.
 */
class OldSpaceSegmentSet : public MemorySegmentSet
{
  public:

    /* the default constructor */
    OldSpaceSegmentSet()  { ; }
    OldSpaceSegmentSet(MemoryObject *memory);
    virtual ~OldSpaceSegmentSet() { ; }

    RexxInternalObject *allocateObject(size_t allocationLength);

    void markOldSpaceObjects();

  protected:
    void addDeadObject(DeadObject *object) override;
    void addDeadObject(char *object, size_t length) override;
    RexxInternalObject *findObject(size_t allocationLength);

  private:
    DeadObjectPool deadCache;             /* the set of objects on the old dead chain */
};

#endif

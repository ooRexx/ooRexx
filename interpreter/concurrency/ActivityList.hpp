/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
#ifndef Included_ActivityList
#define Included_ActivityList

#include <vector>
#include <algorithm>

class Activity;

/**
 * A registry of activities that is deliberately NOT a Rexx object.
 *
 * These registries used to be a QueueClass, which is a Rexx object and is
 * therefore walked by the collector. That is a problem, because the paths that
 * maintain them cannot all hold kernel access:
 *
 *   InterpreterInstance::removeInactiveActivities() runs immediately before
 *   waiting on the termination semaphore, so it cannot hold kernel access -- it
 *   is about to wait for the very threads that need that lock.
 *
 *   ActivityManager::activityEnded() runs after Activity::runThread() has
 *   already released access.
 *
 * They hold the resource lock instead, and the resource lock does not exclude
 * the collector, which runs under the kernel lock. ThreadSanitizer reports the
 * collector marking these lists while another thread splices them.
 *
 * Keeping the container outside the Rexx heap breaks that: the container is
 * plain storage maintained under the resource lock, and the owner marks the
 * activities it holds from its own live() while holding that same lock. GC
 * already holds kernel access, so taking the resource lock there is the
 * permitted lock order, never the reverse.
 *
 * The operations mirror the QueueClass ones that the call sites used, including
 * get() being 1-based, so that converting a call site is only a change of
 * punctuation and cannot silently change an index.
 *
 * IMPORTANT: an ActivityList must never be walked by a handler that is running
 * against a bytewise copy of its owner. MemoryObject::createImage() memcpy's an
 * object into the image buffer and then calls liveGeneral(SAVINGIMAGE) on the
 * *copy*; a QueueClass copy is self-contained, but a copied vector still points
 * at the original's storage, so marking it would write image offsets into the
 * live list. Owners must skip these entries when the reason is SAVINGIMAGE.
 * That constraint is invisible at the call site, which is why it is stated here.
 */
class ActivityList
{
    // contents() hands out the raw vector, so only the two owners that mark it
    // are allowed to reach it. Anything else must go through the operations.
    friend class ActivityManager;
    friend class InterpreterInstance;

public:
    ActivityList() { }

    // A copy would leave two objects sharing one buffer. Nothing copies these
    // today; deleting the operations keeps it that way by construction rather
    // than by review.
    ActivityList(const ActivityList &) = delete;
    ActivityList &operator=(const ActivityList &) = delete;

    // append to the end of the list
    inline void append(Activity *activity) { activities.push_back(activity); }

    // number of entries currently held
    inline size_t items() const { return activities.size(); }
    inline bool isEmpty() const { return activities.empty(); }
    // the last valid index, which for a queue is the same as the item count
    inline size_t lastIndex() const { return activities.size(); }

    /**
     * Fetch an entry. Indexes are 1-based, as they were on QueueClass.
     *
     * @param index The 1-based index.
     *
     * @return The activity, or NULL if the index is out of range.
     */
    inline Activity *get(size_t index) const
    {
        if (index < 1 || index > activities.size())
        {
            return NULL;
        }
        return activities[index - 1];
    }

    /**
     * Remove and return the entry at the front of the list.
     *
     * @return The first activity, or NULL if the list is empty.
     */
    inline Activity *pull()
    {
        if (activities.empty())
        {
            return NULL;
        }
        Activity *activity = activities.front();
        activities.erase(activities.begin());
        return activity;
    }

    /**
     * Remove a specific activity from anywhere in the list.
     *
     * @param activity The activity to remove. Not being present is not an error,
     *                 which matches how QueueClass::removeItem() behaved here.
     */
    inline void removeItem(Activity *activity)
    {
        std::vector<Activity *>::iterator it = std::find(activities.begin(), activities.end(), activity);
        if (it != activities.end())
        {
            activities.erase(it);
        }
    }

    inline void clear() { activities.clear(); }

    /**
     * Release the storage, not just the contents.
     *
     * InterpreterInstance is a Rexx heap object: its operator delete is empty
     * and swept objects never run a C++ destructor, so ~vector never runs and
     * clear() alone would leak the buffer for the life of the process. An
     * embedder creating and terminating instances in a loop would leak without
     * bound.
     */
    inline void reset() { std::vector<Activity *>().swap(activities); }

protected:

    // Direct access for the owners' live()/liveGeneral(), which must mark every
    // entry, and for liveGeneral must write the entry back. The caller holds the
    // resource lock, or kernel access, while doing so.
    inline std::vector<Activity *> &contents() { return activities; }

private:

    std::vector<Activity *> activities;
};

#endif

/**************************************************************************
 * Copyright 2017-2018 NextCash, LLC                                      *
 * Contributors :                                                         *
 *   Curtis Ellis <curtis@nextcash.tech>                                  *
 * Distributed under the MIT software license, see the accompanying       *
 * file license.txt or http://www.opensource.org/licenses/mit-license.php *
 **************************************************************************/
#ifndef NEXTCASH_PROFILER_HPP
#define NEXTCASH_PROFILER_HPP

#include <cstdint>

#include "string.hpp"
#include "log.hpp"
#include "mutex.hpp"
#include "timer.hpp"


namespace NextCash
{
#ifdef PROFILER_ON
    static const unsigned int PROFILER_SET = 0;
    static unsigned int sNextID = 0;

    static const unsigned int PROFILER_HASH_FILE_SET_SUB_SAVE_ID = sNextID++;
    static const char *PROFILER_HASH_FILE_SET_SUB_SAVE_NAME __attribute__ ((unused)) = "HashDataFileSet::SubSet::save";
    static const unsigned int PROFILER_HASH_FILE_SET_INSERT_ID = sNextID++;
    static const char *PROFILER_HASH_FILE_SET_INSERT_NAME __attribute__ ((unused)) = "HashDataFileSet::insert";
    static const unsigned int PROFILER_HASH_FILE_SET_PULL_ID = sNextID++;
    static const char *PROFILER_HASH_FILE_SET_PULL_NAME __attribute__ ((unused)) = "HashDataFileSet::SubSet::pull";

    static const unsigned int PROFILER_HASH_SET_FIND_ID = sNextID++;
    static const char *PROFILER_HASH_SET_FIND_NAME __attribute__ ((unused)) = "HashSet::find";

    static const unsigned int PROFILER_SORTED_SET_FIND_ID = sNextID++;
    static const char *PROFILER_SORTED_SET_FIND_NAME __attribute__ ((unused)) = "SortedSet::find";

    static const unsigned int PROFILER_HASH_CONT_FIND_ID = sNextID++;
    static const char *PROFILER_HASH_CONT_FIND_NAME __attribute__ ((unused)) = "HashContainer::findInsertBefore";
    static const unsigned int PROFILER_HASH_CONT_INSERT_ID = sNextID++;
    static const char *PROFILER_HASH_CONT_INSERT_NAME __attribute__ ((unused)) = "HashContainer::insert";
    static const unsigned int PROFILER_HASH_CONT_INSERT_NM_ID = sNextID++;
    static const char *PROFILER_HASH_CONT_INSERT_NM_NAME __attribute__ ((unused)) = "HashContainer::insertNotMatching";
#endif

    class Profiler
    {
    public:

        Profiler(const char *pName) : mName (pName), mHits(0L), mMicroseconds(0L),
          mMutex("Profiler") {}

        const String &name() const { return mName; }
        void setName(const char *pName) { mName = pName; }

        // Add one hit and specified time.
        void addHit(Microseconds pMicroseconds)
        {
            mMutex.lock();
            ++mHits;
            mMicroseconds += pMicroseconds;
            mMutex.unlock();
        }

        // Add hits without incrementing time.
        void addHits(uint64_t pHits)
        {
            mMutex.lock();
            mHits += pHits;
            mMutex.unlock();
        }

        // Add hit without incrementing time.
        void addHit()
        {
            mMutex.lock();
            ++mHits;
            mMutex.unlock();
        }

        // Add time without incrementing hits.
        void addTime(uint64_t pMicroseconds)
        {
            mMutex.lock();
            mMicroseconds += pMicroseconds;
            mMutex.unlock();
        }

        void clear()
        {
            mMutex.lock();
            mHits = 0L;
            mMicroseconds = 0L;
            mMutex.unlock();
        }

        uint64_t hits() const { return mHits; }
        Milliseconds milliseconds() const { return mMicroseconds / 1000L; }
        Microseconds microseconds() const { return mMicroseconds; }

    private:

        String mName;
        uint64_t mHits;
        Microseconds mMicroseconds;
        MutexWithConstantName mMutex;

        Profiler(const Profiler &pCopy) : mMutex("Profiler")
        {
            mName = pCopy.mName;
            mHits = pCopy.mHits;
            mMicroseconds = pCopy.mMicroseconds;
        }

        Profiler &operator = (const Profiler &pRight)
        {
            mName = pRight.mName;
            mHits = pRight.mHits;
            mMicroseconds = pRight.mMicroseconds;
            return *this;
        }

    };

    // Profiler that stops when it goes out of scope.
    class ProfilerReference
    {
    public:
        ProfilerReference(Profiler &pProfiler, bool pStart) : mProfiler(pProfiler)
        {
            mStarted = pStart;
            if(pStart)
                mStartTime = std::chrono::steady_clock::now();
        }

        ~ProfilerReference()
        {
            if(mStarted)
            {
                mProfiler.addHit(std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::steady_clock::now() - mStartTime).count());
            }
        }

        void start()
        {
            mStartTime = std::chrono::steady_clock::now();
            mStarted = true;
        }

        void stop()
        {
            if(!mStarted)
                return;
            mProfiler.addHit(std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now() - mStartTime).count());
            mStarted = false;
        }

    private:
        Profiler &mProfiler;
        bool mStarted;
        std::chrono::steady_clock::time_point mStartTime;

        ProfilerReference(const ProfilerReference &pCopy);
        ProfilerReference &operator = (const ProfilerReference &pRight);
    };

    Profiler &getProfiler(unsigned int pSetID, unsigned int pID, const char *pName);

    void resetProfilers();

    void printProfilerDataToLog(Log::Level pLevel);
}

#endif

/**************************************************************************
 * Copyright 2017 ArcMist, LLC                                            *
 * Contributors :                                                         *
 *   Curtis Ellis <curtis@arcmist.com>                                    *
 * Distributed under the MIT software license, see the accompanying       *
 * file license.txt or http://www.opensource.org/licenses/mit-license.php *
 **************************************************************************/
#ifndef ARCMIST_PROFILER_HPP
#define ARCMIST_PROFILER_HPP

#include "arcmist/io/stream.hpp"

#include <ctime>
#include <chrono>
#include <cstring>
#include <vector>


namespace ArcMist
{
    class ProfilerData
    {
    public:

        ProfilerData(const char *pName) : hits(0), seconds(0.0) { name = pName; started = false; }

        void start() { hits++; begin = std::chrono::steady_clock::now(); started = true; }
        void stop()
        {
            if(!started)
                return;
            seconds += std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - begin).count();
            started = false;
        }
        void write(OutputStream *pStream);

        const char *name;
        std::chrono::steady_clock::time_point begin;
        unsigned long hits;
        bool started;
        double seconds;

    };

    class ProfilerManager
    {
    public:

        static ProfilerData *profilerData(const char *pName)
        {
            std::vector<ProfilerData *> &profilers = instance().mProfilers;
            for(std::vector<ProfilerData *>::iterator iter = profilers.begin();iter!=profilers.end();++iter)
                if(std::strcmp((*iter)->name, pName) == 0)
                    return *iter;

            ProfilerData *newProfiler = new ProfilerData(pName);
            profilers.push_back(newProfiler);
            return newProfiler;
        }

        // Write text results that were collected by profiler
        static void write(OutputStream *pStream);

    private:

        static ProfilerManager *mInstance;
        static void destroy();

        ProfilerManager();
        ~ProfilerManager();

        static ProfilerManager &instance();

        std::vector<ProfilerData *> mProfilers;

    };

    class Profiler
    {
    public:
        Profiler(const char *pName, bool pStart = true)
        {
            mData = ProfilerManager::profilerData(pName);
            if(pStart)
                mData->start();
        }
        ~Profiler() { mData->stop(); }

        void start() { mData->start(); }
        void stop() { mData->stop(); }

        double seconds() { return mData->seconds; }

    private:
        ProfilerData *mData;
    };
}

#endif
#ifndef __INC_ETER_LIB_FILE_LOADER_THREAD_H__
#define __INC_ETER_LIB_FILE_LOADER_THREAD_H__

#include <deque>
#include <vector>
#include <thread>
#include "Thread.h"
#include "Mutex.h"
#include "../EterBase/MappedFile.h"

class CFileLoaderThread
{
public:
    typedef struct SData
    {
        std::string stFileName;
        CMappedFile File;
        LPVOID pvBuf;
        DWORD dwSize;
    } TData;

public:
    CFileLoaderThread();
    ~CFileLoaderThread();

    int Create(void* arg, size_t threadCount = std::thread::hardware_concurrency());

public:
    void Request(std::string& c_rstFileName);
    bool Fetch(TData** ppData);
    void Shutdown();
    void ReleaseBuffer(char* buf);

protected:
    void Process(TData* pData);

private:
    void WorkerThread();
    void Destroy();

private:
    std::deque<TData*> m_pRequestDeque;
    Mutex m_RequestMutex;

    std::deque<TData*> m_pCompleteDeque;
    Mutex m_CompleteMutex;

    std::vector<std::thread> m_threads;
    bool m_bShutdowned;

    std::vector<char*> m_bufferPool;
    Mutex m_bufferPoolMutex;

    char* AllocateBuffer(size_t size);
};

#endif // __INC_ETER_LIB_FILE_LOADER_THREAD_H__
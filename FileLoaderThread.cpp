#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"
#include "FileLoaderThread.h"
#include "ResourceManager.h"

CFileLoaderThread::CFileLoaderThread() : m_bShutdowned(false)
{
}

CFileLoaderThread::~CFileLoaderThread()
{
    Destroy();
}

int CFileLoaderThread::Create(void* /*arg*/, size_t threadCount)
{
    m_bShutdowned = false;
    for (size_t i = 0; i < threadCount; ++i)
    {
        m_threads.emplace_back(&CFileLoaderThread::WorkerThread, this);
    }
    return true;
}

void CFileLoaderThread::WorkerThread()
{
    while (!m_bShutdowned)
    {
        std::vector<TData*> batch;
        {
            m_RequestMutex.Lock();
            size_t batchSize = std::min<size_t>(m_pRequestDeque.size(), 5); // Proceseaza maxim 5 cereri simultan
            for (size_t i = 0; i < batchSize; ++i)
            {
                batch.push_back(m_pRequestDeque.front());
                m_pRequestDeque.pop_front();
            }
            m_RequestMutex.Unlock();
        }

        for (TData* pData : batch)
        {
            Process(pData);
            {
                m_CompleteMutex.Lock();
                m_pCompleteDeque.push_back(pData);
                m_CompleteMutex.Unlock();
            }
        }

        if (batch.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void CFileLoaderThread::Process(TData* pData)
{
    LPCVOID pvBuf;
    if (CEterPackManager::Instance().Get(pData->File, pData->stFileName.c_str(), &pvBuf))
    {
        pData->dwSize = pData->File.Size();
        pData->pvBuf = AllocateBuffer(pData->dwSize);
        memcpy(pData->pvBuf, pvBuf, pData->dwSize);
    }
}

void CFileLoaderThread::Request(std::string& c_rstFileName)
{
    TData* pData = new TData;
    pData->dwSize = 0;
    pData->pvBuf = nullptr;
    pData->stFileName = c_rstFileName;

    m_RequestMutex.Lock();
    m_pRequestDeque.push_back(pData);
    m_RequestMutex.Unlock();
}

char* CFileLoaderThread::AllocateBuffer(size_t size)
{
    m_bufferPoolMutex.Lock();
    for (auto it = m_bufferPool.begin(); it != m_bufferPool.end(); ++it)
    {
        char* buf = *it;
        if (buf)
        {
            m_bufferPool.erase(it);
            m_bufferPoolMutex.Unlock();
            return buf;
        }
    }
    m_bufferPoolMutex.Unlock();
    return new char[size];
}

void CFileLoaderThread::ReleaseBuffer(char* buf)
{
    m_bufferPoolMutex.Lock();
    m_bufferPool.push_back(buf);
    m_bufferPoolMutex.Unlock();
}

bool CFileLoaderThread::Fetch(TData** ppData)
{
    m_CompleteMutex.Lock();
    if (m_pCompleteDeque.empty())
    {
        m_CompleteMutex.Unlock();
        return false;
    }

    *ppData = m_pCompleteDeque.front();
    m_pCompleteDeque.pop_front();
    m_CompleteMutex.Unlock();
    return true;
}

void CFileLoaderThread::Shutdown()
{
    m_bShutdowned = true;
    for (auto& thread : m_threads)
    {
        if (thread.joinable())
            thread.join();
    }
    Destroy();
}

void CFileLoaderThread::Destroy()
{
    stl_wipe(m_pRequestDeque);
    stl_wipe(m_pCompleteDeque);
    m_threads.clear();
}
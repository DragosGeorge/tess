#pragma once

#include "Resource.h"
#include "FileLoaderThread.h"

#include <set>
#include <map>
#include <string>

#if defined(ENABLE_LOADING_PERFORMANCE)
#	include <mutex>
#endif

#include <unordered_map>

class CResourceManager : public CSingleton<CResourceManager>
{
public:
	CResourceManager();
	virtual ~CResourceManager();

	void LoadStaticCache(const char* c_szFileName);

	void DestroyDeletingList();
	void Destroy();

	void BeginThreadLoading();
	void EndThreadLoading();

	CResource* InsertResourcePointer(DWORD dwFileCRC, CResource* pResource);
	CResource* FindResourcePointer(DWORD dwFileCRC);
	CResource* GetResourcePointer(const char* c_szFileName);
	CResource* GetTypeResourcePointer(const char* c_szFileName, int iType = -1);

	// 추가
	bool isResourcePointerData(DWORD dwFileCRC);

	void RegisterResourceNewFunctionPointer(const char* c_szFileExt, CResource* (*pResNewFunc)(const char* c_szFileName));
	void RegisterResourceNewFunctionByTypePointer(int iType, CResource* (*pNewFunc) (const char* c_szFileName));

	void DumpFileListToTextFile(const char* c_szFileName);
	bool IsFileExist(const char* c_szFileName);

	void Update();
	void ReserveDeletingResource(CResource* pResource);

	float GetPriority(const std::string& fileName, const D3DXVECTOR3& playerPosition, const std::map<std::string, D3DXVECTOR3>& filePositions);

public:

	void ProcessBackgroundLoading();
	/*void PushBackgroundLoadingSet(std::set<std::string>& LoadingSet);*/
	void PushBackgroundLoadingSet(std::set<std::string>& LoadingSet, const D3DXVECTOR3& playerPosition, const std::map<std::string, D3DXVECTOR3>& filePositions);

protected:
	void __DestroyDeletingResourceMap();
	void __DestroyResourceMap();
	void __DestroyCacheMap();

	DWORD __GetFileCRC(const char* c_szFileName, const char** c_pszLowerFile = NULL);

	std::unordered_map<std::string, bool> m_fileExistCache;

protected:
	typedef std::unordered_map<DWORD, CResource*> TResourcePointerMap;
	typedef std::unordered_map<std::string, CResource* (*)(const char*)> TResourceNewFunctionPointerMap;
	typedef std::unordered_map<int, CResource* (*)(const char*)> TResourceNewFunctionByTypePointerMap;
	typedef std::unordered_map<CResource*, DWORD> TResourceDeletingMap;
	typedef std::unordered_map<DWORD, std::string> TResourceRequestMap;
	typedef std::unordered_map<long, CResource*> TResourceRefDecreaseWaitingMap;

protected:
	TResourcePointerMap m_pCacheMap;
	TResourcePointerMap m_pResMap;
	TResourceNewFunctionPointerMap m_pResNewFuncMap;
	TResourceNewFunctionByTypePointerMap m_pResNewFuncByTypeMap;
	TResourceDeletingMap m_ResourceDeletingMap;
	TResourceRequestMap m_RequestMap; // 쓰레드로 로딩 요청한 리스트
	TResourceRequestMap m_WaitingMap;
	TResourceRefDecreaseWaitingMap m_pResRefDecreaseWaitingMap;

	static CFileLoaderThread ms_loadingThread;
#if defined(ENABLE_LOADING_PERFORMANCE)
	static std::mutex						ms_mutex;
#endif
};

extern int g_iLoadingDelayTime;

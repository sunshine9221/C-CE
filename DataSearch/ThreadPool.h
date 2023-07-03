#pragma once
#include "Thread.h"
#include <vector>
#include <memory>

namespace edoyun {
	using FuncPointer = std::shared_ptr<CFunctionBase>;
	class Task {
	public:
		OVERLAPPED Overlapped;
		std::atomic<FuncPointer*> TaskFunction;
	public:
		template< typename _FX, typename... _Types >
		Task(_FX _Func, _Types... _Args) {
			TaskFunction = new FuncPointer(new CFunction<_FX, _Types...>(_Func, _Args...));
			memset(&Overlapped, 0, sizeof(Overlapped));
		}
		Task(const Task& task);
		~Task();
		Task& operator=(const Task& task);
	};
	class ThreadPool
	{
	public:
		ThreadPool();
		virtual ~ThreadPool();
		int Start(DWORD nThreadCount);
		int AddTask(const Task& task);
		int Close();
	private:
		int TaskExcute();
		void ClearIocpQueue();
	private:
		std::vector<CThread*> m_vecThreads;
		HANDLE m_hIocp;
		int m_nStatus;//0 表示正常 1 表示即将关闭
	};
}

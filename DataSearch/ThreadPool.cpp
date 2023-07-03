#include "pch.h"
#include "framework.h"
#include "ThreadPool.h"
using namespace edoyun;

edoyun::Task::Task(const Task& task)
{
	memcpy(&Overlapped, &task.Overlapped, sizeof(task.Overlapped));
	TaskFunction = new FuncPointer(*(FuncPointer*)task.TaskFunction);
}

edoyun::Task::~Task()
{
	FuncPointer* pFunc = std::atomic_exchange(&TaskFunction, NULL);
	if (pFunc != NULL) {
		pFunc->reset();
		delete pFunc;
	}
}

Task& edoyun::Task::operator=(const Task& task)
{
	if (this != &task) {
		memcpy(&Overlapped, &task.Overlapped, sizeof(task.Overlapped));
		TaskFunction = new FuncPointer(*(FuncPointer*)task.TaskFunction);
	}
	return *this;
}

edoyun::ThreadPool::ThreadPool()
{
	m_hIocp = NULL;
	m_nStatus = 0;
}

edoyun::ThreadPool::~ThreadPool()
{
	Close();
}

int edoyun::ThreadPool::Start(DWORD nThreadCount)
{
	if (m_hIocp != NULL)return -1;
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, nThreadCount);
	if (m_hIocp == NULL)return -2;
	m_vecThreads.resize(nThreadCount);
	if (m_vecThreads.size() != nThreadCount)
		return -3;
	int ret = 0;
	for (DWORD i = 0; i < nThreadCount; i++) {
		m_vecThreads[i] = new CThread();
		ret = m_vecThreads[i]->SetThreadFunc(&ThreadPool::TaskExcute, this);
		if (ret != 0) {
			Close();
			return -4;
		}
		ret = m_vecThreads[i]->Start();
		if (ret != 0) {
			Close();
			return -5;
		}
	}
	return 0;
}

int edoyun::ThreadPool::AddTask(const Task& task)
{
	if (m_hIocp != NULL && m_nStatus == 0) {
		Task* pTask = new Task(task);
		if (PostQueuedCompletionStatus(m_hIocp, 1, 0, &pTask->Overlapped) == FALSE) {
			delete pTask;
			return -1;
		}
	}
	else {
		return -2;
	}
	return 0;
}

int edoyun::ThreadPool::Close()
{
	if (m_nStatus == 0) {
		m_nStatus = 1;
	}
	if (m_hIocp != NULL) {
		PostQueuedCompletionStatus(m_hIocp, 0, 0, NULL);
		WaitForSingleObject(m_hIocp, 100);
		CloseHandle(m_hIocp);
		m_hIocp = NULL;
	}
	if (m_vecThreads.size() > 0) {
		DWORD nCount = m_vecThreads.size();
		for (DWORD i = 0; i < nCount; i++) {
			m_vecThreads[i]->Stop();
			delete m_vecThreads[i];
			m_vecThreads[i] = NULL;
		}
		m_vecThreads.clear();
	}
	return 0;
}

int edoyun::ThreadPool::TaskExcute()
{
	DWORD NumberOfBytesTransferred;
	ULONG_PTR CompletionKey;
	LPOVERLAPPED lpOverlapped;
	while (m_hIocp != NULL) {
		if (m_nStatus == 1) {
			//马上要退出了，先把队列里面的信息都给清理了
			ClearIocpQueue();
			break;
		}
		DWORD ret = GetQueuedCompletionStatus(m_hIocp,
			&NumberOfBytesTransferred,
			&CompletionKey,
			&lpOverlapped, INFINITE);
		if (ret == 0)break;
		//结束退出
		if (NumberOfBytesTransferred == 0 &&
			(CompletionKey == NULL))
		{//马上要退出了，先把队列里面的信息都给清理了
			ClearIocpQueue();
			break;
		}
		if (lpOverlapped != NULL) {
			Task* pTask = CONTAINING_RECORD(lpOverlapped, Task, Overlapped);
			if (pTask && pTask->TaskFunction.load()) {
				if (*pTask->TaskFunction.load()) {
					FuncPointer* func = pTask->TaskFunction.load();
					(*(*func))();
				}
				delete pTask;
			}
		}
	}
	return 0;
}

void edoyun::ThreadPool::ClearIocpQueue()
{
	DWORD NumberOfBytesTransferred;
	ULONG_PTR CompletionKey;
	LPOVERLAPPED lpOverlapped;
	while (GetQueuedCompletionStatus(m_hIocp,
		&NumberOfBytesTransferred,
		&CompletionKey,
		&lpOverlapped, 0)) {
		if (lpOverlapped != NULL)
		{
			delete CONTAINING_RECORD(lpOverlapped, Task, Overlapped);
		}
	}
}

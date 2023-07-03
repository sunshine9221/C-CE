#pragma once
#include <Windows.h>
#include <functional>
#include <memory>
#include <atomic>

namespace edoyun {
	class CFunctionBase
	{
	public:
		virtual ~CFunctionBase() = default;
		virtual int operator()() = 0;
	};

	template <class _FX, class... _Types>
	class CFunction :public CFunctionBase
	{
	public:
		CFunction(_FX _Func, _Types... _Args)
			:m_binder(std::forward<_FX>(_Func), std::forward<_Types>(_Args)...)
		{}
		virtual ~CFunction() {}
		virtual int operator()() {
			return m_binder();
		}
		std::_Binder<std::_Unforced, _FX, _Types...> m_binder;
	};
	enum CThreadStatus
	{
		THREAD_AGAIN = 1,			//线程再次运行
		THREAD_OK = 0,				//线程目前一切正常
		THREAD_IS_INVALID = -1,		//线程无效
		THREAD_PAUSE_ERROR = -2,	//线程暂停错误
		THREAD_IS_BUSY = -3,		//线程占线，被强制结束
	};

	class CThread
	{
	public://std::bind
		template< typename _FX, typename... _Types >
		CThread(_FX _Func, _Types... _Args) :m_pFunction(new CFunction<_FX, _Types...>(_Func, _Args...))
		{
			m_bValid = false;
			m_hThread = CreateThread(
				NULL, 0, CThread::ThreadEntry,
				this, CREATE_SUSPENDED, &m_nThreadID);
		}
		CThread();
		~CThread();
		//禁止复制！！！
		CThread(const CThread&) = delete;
		CThread& operator=(const CThread&) = delete;
		int Start();//开始/恢复
		int Pause();//暂停
		int Stop();//停止
		int Restart();//重启线程（上一次运行必须结束）
		bool isValid() const;
		template< typename _FX, typename... _Types >
		int SetThreadFunc(_FX _Func, _Types... _Args) {
			if (_Func == NULL) {
				m_pFunction = NULL;
			}
			else {
				m_pFunction = new CFunction<_FX, _Types...>(_Func, _Args...);
			}
			return 0;
		}
	private:
		static DWORD WINAPI ThreadEntry(LPVOID lpParam);
		void EnterThread();//__thiscall
	private:
		DWORD m_nThreadID;
		HANDLE m_hThread;
		bool m_bValid;//True表示线程正常 False表示线程不可用
		std::atomic<CFunctionBase*> m_pFunction;
	};
}

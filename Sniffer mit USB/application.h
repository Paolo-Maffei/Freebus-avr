
#ifndef _APPLICATION_H_
#define _APPLICATION_H_


class CApplication : public CTask
{
	public:
		CApplication (void);
		void Init (void);
		void OnShutdown (void);
		bool bRunApplication;


	protected:
		void Execute(void);
};

extern CApplication theApp;

#endif //_APPLICATION_H_

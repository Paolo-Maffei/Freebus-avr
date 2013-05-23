
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

/** 
* @todo add documentation
* 
* 
* @return 
*/
class CApplication : public CTask
{
	public:
		CApplication (void);
		/** @todo add documentation */
          void Init (void);
		/** @todo add documentation */
		void OnShutdown (void);
		/** @todo add documentation */
		bool bRunApplication;


	protected:
		/** @todo add documentation */
		void Execute(void);
};

extern CApplication theApp;

#endif //_APPLICATION_H_

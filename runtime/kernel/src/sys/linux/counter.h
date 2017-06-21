
// This file defines some helpful profiling counter routines.

#ifndef __COUNTER_H__
#define __COUNTER_H__

	#include "ltbasetypes.h"

	void dsi_PrintToConsole(const char *pMsg, ...);


	// You can start a counter in its constructor with these.
	#define CSTART_NONE			0
	#define CSTART_MILLI		1
	#define CSTART_MICRO		2

	
	class CounterFinal
	{
	public:

						CounterFinal(unsigned long startMode=CSTART_NONE);

		// Measure in milliseconds (1/1,000).
		void			StartMS();
		unsigned long	EndMS();
		unsigned long	CountMS();

		// Measure in microseconds (1/1,000,000).
		void			StartMicro();
		unsigned long	EndMicro();
		unsigned long	CountMicro();

		unsigned long m_Data[2];
	};

	typedef CounterFinal Counter;


	// How many ticks per second are there?
	unsigned long cnt_NumTicksPerSecond();

	
	// Start a counter.
	void cnt_StartCounter(Counter &pCounter);

	// Returns the number of ticks since you called StartCounter.
	unsigned long cnt_EndCounter(Counter &pCounter);

	// Start a counter.
	void cnt_StartCounterFinal(CounterFinal &pCounter);

	// Returns the number of ticks since you called StartCounter.
	unsigned long cnt_EndCounterFinal(CounterFinal &pCounter);

	
	// C++ helpers..
	class CountAdder
	{
		public:

			CountAdder(uint32 *pNum)
			{
				m_pNum = pNum;
				cnt_StartCounter(m_Counter);
			}

			~CountAdder()
			{
				*m_pNum += cnt_EndCounter(m_Counter);
			}

			Counter m_Counter;
			uint32 *m_pNum;
	};

	class CountPrinter
	{
	public:
				CountPrinter(char *pMsg)
				{
					m_pMsg = pMsg;
					cnt_StartCounter(m_Counter);
				}

				~CountPrinter()
				{
					dsi_PrintToConsole(m_pMsg, cnt_EndCounter(m_Counter));
				}
		
		char	*m_pMsg;
		Counter	m_Counter;
	};

	class CountPercent
	{
	public:
		CountPercent() : m_iDelayCount(0) { Clear(); };

		unsigned long m_Finger[2];
		unsigned long m_TotalIn[2];
		unsigned long m_TotalOut[2];

		int m_iIn, m_iDelayCount;

		// Calculates the amount of time the process was "in", from 0 to 1.  (returns In/(In+Out))
		float CalcPercent(); 
		// Clears the totals
		void Clear(); 

		// Call to enter the profiled section (returns how long it was "out", unless m_iIn > 0)
		// Note : return may wrap if you leave it out too long
		uint In();
		// Call to exit the profiled section (returns how long it was "in", unless m_iIn > 1)
		// Note : return may wrap if you leave it in too long
		uint Out();

		// Call to actually display the results
		inline void Report(char *pMsg)
		{
			dsi_PrintToConsole(pMsg, CalcPercent() * 100.0f);
			Clear();
		};

		inline void ReportDelayed(char *pMsg, int iDelay)
		{
			if (++m_iDelayCount < iDelay)
				return;
			Report(pMsg);
			m_iDelayCount = 0;
		}
	};

	class CountAutoPercent
	{
	public:
		CountAutoPercent(CountPercent *pCounter) : m_pCounter(pCounter) { m_pCounter->In(); };
		CountAutoPercent(CountPercent *pCounter, char *pMsg, int iDelay = 0) : m_pCounter(pCounter) { m_pCounter->ReportDelayed(pMsg, iDelay); m_pCounter->In(); };
		~CountAutoPercent() { m_pCounter->Out(); };
		CountPercent *m_pCounter;
	};

	
	
#endif  // __COUNTER_H__




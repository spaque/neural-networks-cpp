#pragma once

#include "UmrConfig.h"

/**
	Stopwatch class.
	Represents a simple stopwatch used to measure the
	time taken to finish a track composed of three sectors.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrStopwatch : public UmrObject
{
public:
	UmrStopwatch(void);
	~UmrStopwatch(void);

	void lapFinished();
	void sector1Finished();
	void sector2Finished();

	const ULONG getFirstSectorTime() const;
	const ULONG getSecondSectorTime() const;
	const ULONG getLastSectorTime() const;
	const Ogre::String getCurrentTime() const;
	const Ogre::String getBestTime () const;
	const Ogre::String getLastTime () const;
protected:
	const Ogre::String getStringTime (ULONG time) const;

	/// Best time achieved so far
	ULONG m_bestTime;

	/// Time achieved in the last lap
	ULONG m_lastTime;

	/// Time taken to finish the first sector
	ULONG m_firstSectorTime;

	/// Time taken to finish the second sector
	ULONG m_secondSectorTime;

	/// Time taken to finish the last sector
	ULONG m_lastSectorTime;

	/// Timer used to time events
	TimerPtr m_pTimer;

	/// Number of finished laps
	int	m_laps;
};

typedef boost::shared_ptr<UmrStopwatch> UmrStopwatchPtr;

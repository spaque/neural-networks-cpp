#include "UmrStopwatch.h"

/**
	Constructor.
*/
UmrStopwatch::UmrStopwatch(void) :
	m_bestTime(0), m_lastTime(0),
	m_firstSectorTime(0), m_secondSectorTime(0),
	m_lastSectorTime(0), m_laps(-1)
{
	m_pTimer = TimerPtr(new Ogre::Timer());
}

/**
	Destructor.
*/
UmrStopwatch::~UmrStopwatch(void)
{
}

/**
	Resets stopwatch when the current lap is finished.
*/
void UmrStopwatch::lapFinished()
{
	if (m_laps >= 0) {
		m_lastTime = m_pTimer->getMilliseconds();
		if (m_bestTime == 0) m_bestTime = m_lastTime;
		else if (m_lastTime < m_bestTime) m_bestTime = m_lastTime;
	}
	m_lastSectorTime = 
		m_lastTime - m_firstSectorTime - m_secondSectorTime;

	m_laps++;
	m_pTimer->reset();
}

/**
	Flags the finishing of the first sector.
*/
void UmrStopwatch::sector1Finished()
{
	m_firstSectorTime = m_pTimer->getMilliseconds();
}

/**
	Flags the finishing of the second sector.
*/
void UmrStopwatch::sector2Finished()
{
	m_secondSectorTime = 
		m_pTimer->getMilliseconds() - m_firstSectorTime;
}

/**
	Gets the time taken on the first sector.
*/
const ULONG UmrStopwatch::getFirstSectorTime() const
{
	return m_firstSectorTime;
}

/**
	Gets the time taken on the second sector.
*/
const ULONG UmrStopwatch::getSecondSectorTime() const
{
	return m_secondSectorTime;
}

/**
	Gets the time taken on the last sector.
*/
const ULONG UmrStopwatch::getLastSectorTime() const
{
	return m_lastSectorTime;
}

/**
	Gets the formatted string of a given time.
	@param
		time Time in miliseconds.
*/
const Ogre::String UmrStopwatch::getStringTime(ULONG time) const
{
	std::ostringstream os;
	ULONG minutes, seconds, ms;
	ms = time % 1000;
	seconds = time / 1000;
	minutes = seconds / 60;
	seconds = seconds % 60;
	os << minutes << ":" << 
		seconds << ":" << 
		((ms < 100) ? "0" : "") << ms;
	return Ogre::String(os.str());
}

/**
	Gets the current time of the lap.
*/
const Ogre::String UmrStopwatch::getCurrentTime() const
{
	if (m_laps < 0) return Ogre::String("0:0:0");
	return getStringTime(m_pTimer->getMilliseconds());
}

/**
	Gets the best time achieved so far.
*/
const Ogre::String UmrStopwatch::getBestTime() const
{
	return getStringTime(m_bestTime);
}

/**
	Gets the time of the last lap.
*/
const Ogre::String UmrStopwatch::getLastTime() const
{
	return getStringTime(m_lastTime);
}

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef CHANNEL_ACCESS_MANAGER_H
#define CHANNEL_ACCESS_MANAGER_H

#include <vector>
#include <algorithm>
#include "ns3/event-id.h"
#include "ns3/nstime.h"

namespace ns3 {

class WifiPhy;
class PhyListener;
class Txop;
class MacLow;

/**
 * \brief Manage a set of ns3::Txop
 * \ingroup wifi
 *
 * Handle a set of independent ns3::Txop, each of which represents
 * a single DCF within a MAC stack. Each ns3::Txop has a priority
 * implicitly associated with it (the priority is determined when the
 * ns3::Txop is added to the ChannelAccessManager: the first Txop to be
 * added gets the highest priority, the second, the second highest
 * priority, and so on.) which is used to handle "internal" collisions.
 * i.e., when two local Txop are expected to get access to the
 * medium at the same time, the highest priority local Txop wins
 * access to the medium and the other Txop suffers a "internal"
 * collision.
 */
class ChannelAccessManager : public Object
{
public:
  ChannelAccessManager ();
  virtual ~ChannelAccessManager ();

  /**
   * Set up listener for PHY events.
   *
   * \param phy the WifiPhy to listen to
   */
  void SetupPhyListener (Ptr<WifiPhy> phy);
  /**
   * Remove current registered listener for PHY events.
   *
   * \param phy the WifiPhy to listen to
   */
  void RemovePhyListener (Ptr<WifiPhy> phy);
  /**
   * Set up listener for MacLow events.
   *
   * \param low the MacLow to listen to
   */
  void SetupLow (Ptr<MacLow> low);

  /**
   * \param slotTime the duration of a slot.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSlot (Time slotTime);
  /**
   * \param sifs the duration of a SIFS.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSifs (Time sifs);
  /**
   * \param eifsNoDifs the duration of a EIFS minus the duration of DIFS.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetEifsNoDifs (Time eifsNoDifs);

  /**
   * \return value set previously using SetEifsNoDifs.
   */
  Time GetEifsNoDifs (void) const;

  /**
   * \param txop a new Txop.
   *
   * The ChannelAccessManager does not take ownership of this pointer so, the callee
   * must make sure that the Txop pointer will stay valid as long
   * as the ChannelAccessManager is valid. Note that the order in which Txop
   * objects are added to a ChannelAccessManager matters: the first Txop added
   * has the highest priority, the second Txop added, has the second
   * highest priority, etc.
   */
  void Add (Ptr<Txop> txop);

  /**
   * Determine if a new backoff needs to be generated when a packet is queued
   * for transmission.
   *
   * \param txop the Txop requesting to generate a backoff
   * \return true if backoff needs to be generated, false otherwise
   */
  bool NeedBackoffUponAccess (Ptr<Txop> txop);

  /**
   * \param txop a Txop
   * \param isCfPeriod flag whether it is called during the CF period
   *
   * Notify the ChannelAccessManager that a specific Txop needs access to the
   * medium. The ChannelAccessManager is then responsible for starting an access
   * timer and, invoking Txop::DoNotifyAccessGranted when the access
   * is granted if it ever gets granted.
   */
  void RequestAccess (Ptr<Txop> txop, bool isCfPeriod = false);

  /**
   * \param duration expected duration of reception
   *
   * Notify the Txop that a packet reception started
   * for the expected duration.
   */
  void NotifyRxStartNow (Time duration);
  /**
   * Notify the Txop that a packet reception was just
   * completed successfully.
   */
  void NotifyRxEndOkNow (void);
  /**
   * Notify the Txop that a packet reception was just
   * completed unsuccessfully.
   */
  void NotifyRxEndErrorNow (void);
  /**
   * \param duration expected duration of transmission
   *
   * Notify the Txop that a packet transmission was
   * just started and is expected to last for the specified
   * duration.
   */
  void NotifyTxStartNow (Time duration);
  /**
   * \param duration expected duration of CCA busy period
   *
   * Notify the Txop that a CCA busy period has just started.
   */
  void NotifyMaybeCcaBusyStartNow (Time duration);
  /**
   * \param duration expected duration of channel switching period
   *
   * Notify the Txop that a channel switching period has just started.
   * During switching state, new packets can be enqueued in Txop/QosTxop
   * but they won't access to the medium until the end of the channel switching.
   */
  void NotifySwitchingStartNow (Time duration);
  /**
   * Notify the Txop that the device has been put in sleep mode.
   */
  void NotifySleepNow (void);
  /**
   * Notify the Txop that the device has been put in off mode.
   */
  void NotifyOffNow (void);
  /**
   * Notify the Txop that the device has been resumed from sleep mode.
   */
  void NotifyWakeupNow (void);
  /**
   * Notify the Txop that the device has been resumed from off mode.
   */
  void NotifyOnNow (void);
  /**
   * \param duration the value of the received NAV.
   *
   * Called at end of RX
   */
  void NotifyNavResetNow (Time duration);
  /**
   * \param duration the value of the received NAV.
   *
   * Called at end of RX
   */
  void NotifyNavStartNow (Time duration);
  /**
   * Notify that ack timer has started for the given duration.
   *
   * \param duration the duration of the timer
   */
  void NotifyAckTimeoutStartNow (Time duration);
  /**
   * Notify that ack timer has reset.
   */
  void NotifyAckTimeoutResetNow (void);
  /**
   * Notify that CTS timer has started for the given duration.
   *
   * \param duration the duration of the timer
   */
  void NotifyCtsTimeoutStartNow (Time duration);
  /**
   * Notify that CTS timer has reset.
   */
  void NotifyCtsTimeoutResetNow (void);

  /**
   * Check if the device is busy sending or receiving,
   * or NAV or CCA busy.
   *
   * \return true if the device is busy,
   *         false otherwise
   */
  bool IsBusy (void) const;


protected:
  // Inherited from ns3::Object
  void DoDispose (void);


private:
  /**
   * Update backoff slots for all Txops.
   */
  void UpdateBackoff (void);
  /**
   * Return the most recent time.
   *
   * \param list the initializer list including the times to compare
   *
   * \return the most recent time
   */
  Time MostRecent (std::initializer_list<Time> list) const;
  /**
   * Access will never be granted to the medium _before_
   * the time returned by this method.
   *
   * \param ignoreNav flag whether NAV should be ignored
   *
   * \returns the absolute time at which access could start to be granted
   */
  Time GetAccessGrantStart (bool ignoreNav = false) const;
  /**
   * Return the time when the backoff procedure
   * started for the given Txop.
   *
   * \param txop the Txop
   *
   * \return the time when the backoff procedure started
   */
  Time GetBackoffStartFor (Ptr<Txop> txop);
  /**
   * Return the time when the backoff procedure
   * ended (or will ended) for the given Txop.
   *
   * \param txop the Txop
   *
   * \return the time when the backoff procedure ended (or will ended)
   */
  Time GetBackoffEndFor (Ptr<Txop> txop);

  void DoRestartAccessTimeoutIfNeeded (void);

  /**
   * Called when access timeout should occur
   * (e.g. backoff procedure expired).
   */
  void AccessTimeout (void);
  /**
   * Grant access to Txop using DCF/EDCF contention rules
   */
  void DoGrantDcfAccess (void);
  /**
   * Grant access to Txop using PCF preemption
   *
   * \param txop the Txop
   */
  void DoGrantPcfAccess (Ptr<Txop> txop);

  /**
   * typedef for a vector of Txops
   */
  typedef std::vector<Ptr<Txop>> Txops;

  Txops m_txops;                //!< the vector of managed Txops
  Time m_lastAckTimeoutEnd;     //!< the last Ack timeout end time
  Time m_lastCtsTimeoutEnd;     //!< the last CTS timeout end time
  Time m_lastNavStart;          //!< the last NAV start time
  Time m_lastNavDuration;       //!< the last NAV duration time
  Time m_lastRxStart;           //!< the last receive start time
  Time m_lastRxDuration;        //!< the last receive duration time
  bool m_lastRxReceivedOk;      //!< the last receive OK
  Time m_lastTxStart;           //!< the last transmit start time
  Time m_lastTxDuration;        //!< the last transmit duration time
  Time m_lastBusyStart;         //!< the last busy start time
  Time m_lastBusyDuration;      //!< the last busy duration time
  Time m_lastSwitchingStart;    //!< the last switching start time
  Time m_lastSwitchingDuration; //!< the last switching duration time
  bool m_sleeping;              //!< flag whether it is in sleeping state
  bool m_off;                   //!< flag whether it is in off state
  Time m_eifsNoDifs;            //!< EIFS no DIFS time
  EventId m_accessTimeout;      //!< the access timeout ID
  Time m_slot;                  //!< the slot time
  Time m_sifs;                  //!< the SIFS time
  PhyListener* m_phyListener;   //!< the PHY listener
  Ptr<WifiPhy> m_phy;           //!< pointer to the PHY
};

} //namespace ns3

#endif /* CHANNEL_ACCESS_MANAGER_H */

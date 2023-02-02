/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 *          Stefano Avallone <stavalli@unina.it>
 */

#include "qos-txop.h"

#include "channel-access-manager.h"
#include "ctrl-headers.h"
#include "mac-tx-middle.h"
#include "mgt-headers.h"
#include "mpdu-aggregator.h"
#include "msdu-aggregator.h"
#include "qos-blocked-destinations.h"
#include "wifi-mac-queue.h"
#include "wifi-mac-trailer.h"
#include "wifi-phy.h"
#include "wifi-psdu.h"
#include "wifi-tx-parameters.h"

#include "ns3/ht-frame-exchange-manager.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                                                      \
    if (m_mac)                                                                                     \
    {                                                                                              \
        std::clog << "[mac=" << m_mac->GetAddress() << "] ";                                       \
    }

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("QosTxop");

NS_OBJECT_ENSURE_REGISTERED(QosTxop);

TypeId
QosTxop::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::QosTxop")
            .SetParent<ns3::Txop>()
            .SetGroupName("Wifi")
            .AddConstructor<QosTxop>()
            .AddAttribute("UseExplicitBarAfterMissedBlockAck",
                          "Specify whether explicit BlockAckRequest should be sent upon missed "
                          "BlockAck Response.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&QosTxop::m_useExplicitBarAfterMissedBlockAck),
                          MakeBooleanChecker())
            .AddAttribute("AddBaResponseTimeout",
                          "The timeout to wait for ADDBA response after the Ack to "
                          "ADDBA request is received.",
                          TimeValue(MilliSeconds(1)),
                          MakeTimeAccessor(&QosTxop::SetAddBaResponseTimeout,
                                           &QosTxop::GetAddBaResponseTimeout),
                          MakeTimeChecker())
            .AddAttribute(
                "FailedAddBaTimeout",
                "The timeout after a failed BA agreement. During this "
                "timeout, the originator resumes sending packets using normal "
                "MPDU. After that, BA agreement is reset and the originator "
                "will retry BA negotiation.",
                TimeValue(MilliSeconds(200)),
                MakeTimeAccessor(&QosTxop::SetFailedAddBaTimeout, &QosTxop::GetFailedAddBaTimeout),
                MakeTimeChecker())
            .AddAttribute("BlockAckManager",
                          "The BlockAckManager object.",
                          PointerValue(),
                          MakePointerAccessor(&QosTxop::m_baManager),
                          MakePointerChecker<BlockAckManager>())
            .AddTraceSource("TxopTrace",
                            "Trace source for TXOP start and duration times",
                            MakeTraceSourceAccessor(&QosTxop::m_txopTrace),
                            "ns3::QosTxop::TxopTracedCallback");
    return tid;
}

QosTxop::QosTxop(AcIndex ac)
    : Txop(CreateObject<WifiMacQueue>(ac)),
      m_ac(ac)
{
    NS_LOG_FUNCTION(this);
    m_qosBlockedDestinations = Create<QosBlockedDestinations>();
    m_baManager = CreateObject<BlockAckManager>();
    m_baManager->SetQueue(m_queue);
    m_baManager->SetBlockDestinationCallback(
        MakeCallback(&QosBlockedDestinations::Block, m_qosBlockedDestinations));
    m_baManager->SetUnblockDestinationCallback(
        MakeCallback(&QosBlockedDestinations::Unblock, m_qosBlockedDestinations));
    m_queue->TraceConnectWithoutContext(
        "Expired",
        MakeCallback(&BlockAckManager::NotifyDiscardedMpdu, m_baManager));
}

QosTxop::~QosTxop()
{
    NS_LOG_FUNCTION(this);
}

void
QosTxop::DoDispose()
{
    NS_LOG_FUNCTION(this);
    if (m_baManager)
    {
        m_baManager->Dispose();
    }
    m_baManager = nullptr;
    m_qosBlockedDestinations = nullptr;
    Txop::DoDispose();
}

std::unique_ptr<Txop::LinkEntity>
QosTxop::CreateLinkEntity() const
{
    return std::make_unique<QosLinkEntity>();
}

QosTxop::QosLinkEntity&
QosTxop::GetLink(uint8_t linkId) const
{
    return static_cast<QosLinkEntity&>(Txop::GetLink(linkId));
}

uint8_t
QosTxop::GetQosQueueSize(uint8_t tid, Mac48Address receiver) const
{
    WifiContainerQueueId queueId{WIFI_QOSDATA_UNICAST_QUEUE, receiver, tid};
    uint32_t bufferSize = m_queue->GetNBytes(queueId);
    // A queue size value of 254 is used for all sizes greater than 64 768 octets.
    uint8_t queueSize = static_cast<uint8_t>(std::ceil(std::min(bufferSize, 64769U) / 256.0));
    NS_LOG_DEBUG("Buffer size=" << bufferSize << " Queue Size=" << +queueSize);
    return queueSize;
}

void
QosTxop::SetDroppedMpduCallback(DroppedMpdu callback)
{
    NS_LOG_FUNCTION(this << &callback);
    Txop::SetDroppedMpduCallback(callback);
    m_baManager->SetDroppedOldMpduCallback(callback.Bind(WIFI_MAC_DROP_QOS_OLD_PACKET));
}

void
QosTxop::SetMuCwMin(uint16_t cwMin, uint8_t linkId)
{
    NS_LOG_FUNCTION(this << cwMin << +linkId);
    GetLink(linkId).muCwMin = cwMin;
}

void
QosTxop::SetMuCwMax(uint16_t cwMax, uint8_t linkId)
{
    NS_LOG_FUNCTION(this << cwMax << +linkId);
    GetLink(linkId).muCwMax = cwMax;
}

void
QosTxop::SetMuAifsn(uint8_t aifsn, uint8_t linkId)
{
    NS_LOG_FUNCTION(this << +aifsn << +linkId);
    GetLink(linkId).muAifsn = aifsn;
}

void
QosTxop::SetMuEdcaTimer(Time timer, uint8_t linkId)
{
    NS_LOG_FUNCTION(this << timer << +linkId);
    GetLink(linkId).muEdcaTimer = timer;
}

void
QosTxop::StartMuEdcaTimerNow(uint8_t linkId)
{
    NS_LOG_FUNCTION(this << +linkId);
    auto& link = GetLink(linkId);
    link.muEdcaTimerStartTime = Simulator::Now();
    if (EdcaDisabled(linkId))
    {
        NS_LOG_DEBUG("Disable EDCA for " << link.muEdcaTimer.As(Time::MS));
        m_mac->GetChannelAccessManager(linkId)->DisableEdcaFor(this, link.muEdcaTimer);
    }
}

bool
QosTxop::MuEdcaTimerRunning(uint8_t linkId) const
{
    auto& link = GetLink(linkId);
    return (link.muEdcaTimerStartTime.IsStrictlyPositive() &&
            link.muEdcaTimer.IsStrictlyPositive() &&
            link.muEdcaTimerStartTime + link.muEdcaTimer > Simulator::Now());
}

bool
QosTxop::EdcaDisabled(uint8_t linkId) const
{
    return (MuEdcaTimerRunning(linkId) && GetLink(linkId).muAifsn == 0);
}

uint32_t
QosTxop::GetMinCw(uint8_t linkId) const
{
    if (!MuEdcaTimerRunning(linkId))
    {
        return GetLink(linkId).cwMin;
    }
    NS_ASSERT(!EdcaDisabled(linkId));
    return GetLink(linkId).muCwMin;
}

uint32_t
QosTxop::GetMaxCw(uint8_t linkId) const
{
    if (!MuEdcaTimerRunning(linkId))
    {
        return GetLink(linkId).cwMax;
    }
    NS_ASSERT(!EdcaDisabled(linkId));
    return GetLink(linkId).muCwMax;
}

uint8_t
QosTxop::GetAifsn(uint8_t linkId) const
{
    if (!MuEdcaTimerRunning(linkId))
    {
        return GetLink(linkId).aifsn;
    }
    return GetLink(linkId).muAifsn;
}

Ptr<BlockAckManager>
QosTxop::GetBaManager()
{
    return m_baManager;
}

bool
QosTxop::GetBaAgreementEstablished(Mac48Address address, uint8_t tid) const
{
    return m_baManager->ExistsAgreementInState(address,
                                               tid,
                                               OriginatorBlockAckAgreement::ESTABLISHED);
}

uint16_t
QosTxop::GetBaBufferSize(Mac48Address address, uint8_t tid) const
{
    return m_baManager->GetRecipientBufferSize(address, tid);
}

uint16_t
QosTxop::GetBaStartingSequence(Mac48Address address, uint8_t tid) const
{
    return m_baManager->GetOriginatorStartingSequence(address, tid);
}

Ptr<const WifiMpdu>
QosTxop::PrepareBlockAckRequest(Mac48Address recipient, uint8_t tid) const
{
    NS_LOG_FUNCTION(this << recipient << +tid);
    NS_ASSERT(QosUtilsMapTidToAc(tid) == m_ac);

    CtrlBAckRequestHeader reqHdr = m_baManager->GetBlockAckReqHeader(recipient, tid);
    Ptr<Packet> bar = Create<Packet>();
    bar->AddHeader(reqHdr);

    WifiMacHeader hdr;
    hdr.SetType(WIFI_MAC_CTL_BACKREQ);
    hdr.SetAddr1(recipient);
    hdr.SetAddr2(m_mac->GetAddress());
    hdr.SetDsNotTo();
    hdr.SetDsNotFrom();
    hdr.SetNoRetry();
    hdr.SetNoMoreFragments();

    return Create<const WifiMpdu>(bar, hdr);
}

void
QosTxop::ScheduleBar(Ptr<const WifiMpdu> bar, bool skipIfNoDataQueued)
{
    m_baManager->ScheduleBar(bar, skipIfNoDataQueued);
}

bool
QosTxop::UseExplicitBarAfterMissedBlockAck() const
{
    return m_useExplicitBarAfterMissedBlockAck;
}

bool
QosTxop::HasFramesToTransmit(uint8_t linkId)
{
    // check if the BA manager has anything to send, so that expired
    // frames (if any) are removed and a BlockAckRequest is scheduled to advance
    // the starting sequence number of the transmit (and receiver) window
    bool baManagerHasPackets{m_baManager->GetBar(false)};
    // remove MSDUs with expired lifetime starting from the head of the queue
    m_queue->WipeAllExpiredMpdus();
    bool queueIsNotEmpty = (bool)(m_queue->PeekFirstAvailable(linkId, m_qosBlockedDestinations));

    NS_LOG_FUNCTION(this << baManagerHasPackets << queueIsNotEmpty);
    return baManagerHasPackets || queueIsNotEmpty;
}

uint16_t
QosTxop::GetNextSequenceNumberFor(const WifiMacHeader* hdr)
{
    return m_txMiddle->GetNextSequenceNumberFor(hdr);
}

uint16_t
QosTxop::PeekNextSequenceNumberFor(const WifiMacHeader* hdr)
{
    return m_txMiddle->PeekNextSequenceNumberFor(hdr);
}

bool
QosTxop::IsQosOldPacket(Ptr<const WifiMpdu> mpdu)
{
    NS_LOG_FUNCTION(this << *mpdu);

    if (!mpdu->GetHeader().IsQosData())
    {
        return false;
    }

    Mac48Address recipient = mpdu->GetHeader().GetAddr1();
    uint8_t tid = mpdu->GetHeader().GetQosTid();

    if (!GetBaAgreementEstablished(recipient, tid))
    {
        return false;
    }

    if (QosUtilsIsOldPacket(GetBaStartingSequence(recipient, tid),
                            mpdu->GetHeader().GetSequenceNumber()))
    {
        return true;
    }
    return false;
}

Ptr<WifiMpdu>
QosTxop::PeekNextMpdu(uint8_t linkId, uint8_t tid, Mac48Address recipient, Ptr<WifiMpdu> item)
{
    NS_LOG_FUNCTION(this << +linkId << +tid << recipient << item);

    // lambda to peek the next frame
    auto peek = [this, &linkId, &tid, &recipient, &item]() -> Ptr<WifiMpdu> {
        if (tid == 8 && recipient.IsBroadcast()) // undefined TID and recipient
        {
            return m_queue->PeekFirstAvailable(linkId, m_qosBlockedDestinations, item);
        }
        if (m_qosBlockedDestinations->IsBlocked(recipient, tid))
        {
            return nullptr;
        }
        return m_queue->PeekByTidAndAddress(tid, recipient, item);
    };

    item = peek();
    // remove old packets (must be retransmissions or in flight, otherwise they did
    // not get a sequence number assigned)
    while (item && !item->IsFragment())
    {
        if ((item->GetHeader().IsRetry() || item->IsInFlight()) && IsQosOldPacket(item))
        {
            NS_LOG_DEBUG("Removing an old packet from EDCA queue: " << *item);
            if (!m_droppedMpduCallback.IsNull())
            {
                m_droppedMpduCallback(WIFI_MAC_DROP_QOS_OLD_PACKET, item);
            }
            auto oldItem = item;
            item = peek();
            m_queue->Remove(oldItem);
        }
        else if (item->IsInFlight())
        {
            NS_LOG_DEBUG("Skipping in flight MPDU: " << *item);
            item = peek();
        }
        else if (item->GetHeader().HasData() &&
                 !m_mac->CanForwardPacketsTo(item->GetHeader().GetAddr1()))
        {
            NS_LOG_DEBUG("Skipping frame that cannot be forwarded: " << *item);
            item = peek();
        }
        else
        {
            break;
        }
    }
    if (item)
    {
        NS_ASSERT(!item->IsInFlight());
        WifiMacHeader& hdr = item->GetHeader();

        // peek the next sequence number and check if it is within the transmit window
        // in case of QoS data frame
        uint16_t sequence =
            (hdr.IsRetry() ? hdr.GetSequenceNumber() : m_txMiddle->PeekNextSequenceNumberFor(&hdr));
        if (hdr.IsQosData())
        {
            Mac48Address recipient = hdr.GetAddr1();
            uint8_t tid = hdr.GetQosTid();

            if (GetBaAgreementEstablished(recipient, tid) &&
                !IsInWindow(sequence,
                            GetBaStartingSequence(recipient, tid),
                            GetBaBufferSize(recipient, tid)))
            {
                NS_LOG_DEBUG("Packet beyond the end of the current transmit window");
                return nullptr;
            }
        }

        // Assign a sequence number if this is not a fragment nor a retransmission
        if (!item->IsFragment() && !hdr.IsRetry())
        {
            hdr.SetSequenceNumber(sequence);
        }
        NS_LOG_DEBUG("Packet peeked from EDCA queue: " << *item);
        return item;
    }

    return nullptr;
}

Ptr<WifiMpdu>
QosTxop::GetNextMpdu(uint8_t linkId,
                     Ptr<WifiMpdu> peekedItem,
                     WifiTxParameters& txParams,
                     Time availableTime,
                     bool initialFrame)
{
    NS_ASSERT(peekedItem);
    NS_LOG_FUNCTION(this << +linkId << *peekedItem << &txParams << availableTime << initialFrame);

    Mac48Address recipient = peekedItem->GetHeader().GetAddr1();

    // The TXOP limit can be exceeded by the TXOP holder if it does not transmit more
    // than one Data or Management frame in the TXOP and the frame is not in an A-MPDU
    // consisting of more than one MPDU (Sec. 10.22.2.8 of 802.11-2016)
    Time actualAvailableTime =
        (initialFrame && txParams.GetSize(recipient) == 0 ? Time::Min() : availableTime);

    auto qosFem = StaticCast<QosFrameExchangeManager>(m_mac->GetFrameExchangeManager(linkId));
    if (!qosFem->TryAddMpdu(peekedItem, txParams, actualAvailableTime))
    {
        return nullptr;
    }

    NS_ASSERT(peekedItem->IsQueued());
    Ptr<WifiMpdu> mpdu;

    // If it is a non-broadcast QoS Data frame and it is not a retransmission nor a fragment,
    // attempt A-MSDU aggregation
    if (peekedItem->GetHeader().IsQosData())
    {
        uint8_t tid = peekedItem->GetHeader().GetQosTid();

        // we should not be asked to dequeue an MPDU that is beyond the transmit window.
        // Note that PeekNextMpdu() temporarily assigns the next available sequence number
        // to the peeked frame
        NS_ASSERT(!GetBaAgreementEstablished(recipient, tid) ||
                  IsInWindow(peekedItem->GetHeader().GetSequenceNumber(),
                             GetBaStartingSequence(recipient, tid),
                             GetBaBufferSize(recipient, tid)));

        // try A-MSDU aggregation
        if (m_mac->GetHtSupported() && !recipient.IsBroadcast() &&
            !peekedItem->GetHeader().IsRetry() && !peekedItem->IsFragment() &&
            !peekedItem->IsInFlight())
        {
            auto htFem = StaticCast<HtFrameExchangeManager>(qosFem);
            mpdu = htFem->GetMsduAggregator()->GetNextAmsdu(peekedItem, txParams, availableTime);
        }

        if (mpdu)
        {
            NS_LOG_DEBUG("Prepared an MPDU containing an A-MSDU");
        }
        // else aggregation was not attempted or failed
    }

    if (!mpdu)
    {
        mpdu = peekedItem;
    }

    // Assign a sequence number if this is not a fragment nor a retransmission
    AssignSequenceNumber(mpdu);
    NS_LOG_DEBUG("Got MPDU from EDCA queue: " << *mpdu);

    return mpdu;
}

void
QosTxop::AssignSequenceNumber(Ptr<WifiMpdu> mpdu) const
{
    NS_LOG_FUNCTION(this << *mpdu);

    if (!mpdu->IsFragment() && !mpdu->GetHeader().IsRetry() && !mpdu->IsInFlight())
    {
        uint16_t sequence = m_txMiddle->GetNextSequenceNumberFor(&mpdu->GetHeader());
        mpdu->GetHeader().SetSequenceNumber(sequence);
    }
}

BlockAckReqType
QosTxop::GetBlockAckReqType(Mac48Address recipient, uint8_t tid) const
{
    return m_baManager->GetBlockAckReqType(recipient, tid);
}

BlockAckType
QosTxop::GetBlockAckType(Mac48Address recipient, uint8_t tid) const
{
    return m_baManager->GetBlockAckType(recipient, tid);
}

void
QosTxop::NotifyChannelAccessed(uint8_t linkId, Time txopDuration)
{
    NS_LOG_FUNCTION(this << +linkId << txopDuration);

    NS_ASSERT(txopDuration != Time::Min());
    GetLink(linkId).startTxop = Simulator::Now();
    GetLink(linkId).txopDuration = txopDuration;
    Txop::NotifyChannelAccessed(linkId);
}

bool
QosTxop::IsTxopStarted(uint8_t linkId) const
{
    auto& link = GetLink(linkId);
    NS_LOG_FUNCTION(this << !link.startTxop.IsZero());
    return (!link.startTxop.IsZero());
}

void
QosTxop::NotifyChannelReleased(uint8_t linkId)
{
    NS_LOG_FUNCTION(this << +linkId);
    auto& link = GetLink(linkId);

    if (link.startTxop.IsStrictlyPositive())
    {
        NS_LOG_DEBUG("Terminating TXOP. Duration = " << Simulator::Now() - link.startTxop);
        m_txopTrace(link.startTxop, Simulator::Now() - link.startTxop, linkId);
    }
    link.startTxop = Seconds(0);
    Txop::NotifyChannelReleased(linkId);
}

Time
QosTxop::GetRemainingTxop(uint8_t linkId) const
{
    auto& link = GetLink(linkId);
    NS_ASSERT(link.startTxop.IsStrictlyPositive());

    Time remainingTxop = link.txopDuration;
    remainingTxop -= (Simulator::Now() - link.startTxop);
    if (remainingTxop.IsStrictlyNegative())
    {
        remainingTxop = Seconds(0);
    }
    NS_LOG_FUNCTION(this << remainingTxop);
    return remainingTxop;
}

void
QosTxop::GotAddBaResponse(const MgtAddBaResponseHeader* respHdr, Mac48Address recipient)
{
    NS_LOG_FUNCTION(this << respHdr << recipient);
    uint8_t tid = respHdr->GetTid();
    if (respHdr->GetStatusCode().IsSuccess())
    {
        NS_LOG_DEBUG("block ack agreement established with " << recipient << " tid " << +tid);
        // A (destination, TID) pair is "blocked" (i.e., no more packets are sent)
        // when an Add BA Request is sent to the destination. However, when the
        // Add BA Request timer expires, the (destination, TID) pair is "unblocked"
        // and packets to the destination are sent again (under normal ack policy).
        // Thus, there may be a packet needing to be retransmitted when the
        // Add BA Response is received. In this case, the starting sequence number
        // shall be set equal to the sequence number of such packet.
        uint16_t startingSeq = m_txMiddle->GetNextSeqNumberByTidAndAddress(tid, recipient);
        auto peekedItem = m_queue->PeekByTidAndAddress(tid, recipient);
        if (peekedItem && peekedItem->GetHeader().IsRetry())
        {
            startingSeq = peekedItem->GetHeader().GetSequenceNumber();
        }
        m_baManager->UpdateAgreement(respHdr, recipient, startingSeq);
    }
    else
    {
        NS_LOG_DEBUG("discard ADDBA response" << recipient);
        m_baManager->NotifyAgreementRejected(recipient, tid);
    }

    if (HasFramesToTransmit(SINGLE_LINK_OP_ID) &&
        GetLink(SINGLE_LINK_OP_ID).access == NOT_REQUESTED)
    {
        m_mac->GetChannelAccessManager(SINGLE_LINK_OP_ID)->RequestAccess(this);
    }
}

void
QosTxop::GotDelBaFrame(const MgtDelBaHeader* delBaHdr, Mac48Address recipient)
{
    NS_LOG_FUNCTION(this << delBaHdr << recipient);
    NS_LOG_DEBUG("received DELBA frame from=" << recipient);
    m_baManager->DestroyAgreement(recipient, delBaHdr->GetTid());
}

void
QosTxop::CompleteMpduTx(Ptr<WifiMpdu> mpdu)
{
    NS_ASSERT(mpdu->GetHeader().IsQosData());
    // If there is an established BA agreement, store the packet in the queue of outstanding packets
    if (GetBaAgreementEstablished(mpdu->GetHeader().GetAddr1(), mpdu->GetHeader().GetQosTid()))
    {
        m_baManager->StorePacket(mpdu);
    }
}

void
QosTxop::SetBlockAckThreshold(uint8_t threshold)
{
    NS_LOG_FUNCTION(this << +threshold);
    m_blockAckThreshold = threshold;
    m_baManager->SetBlockAckThreshold(threshold);
}

void
QosTxop::SetBlockAckInactivityTimeout(uint16_t timeout)
{
    NS_LOG_FUNCTION(this << timeout);
    m_blockAckInactivityTimeout = timeout;
}

uint8_t
QosTxop::GetBlockAckThreshold() const
{
    NS_LOG_FUNCTION(this);
    return m_blockAckThreshold;
}

uint16_t
QosTxop::GetBlockAckInactivityTimeout() const
{
    return m_blockAckInactivityTimeout;
}

void
QosTxop::AddBaResponseTimeout(Mac48Address recipient, uint8_t tid)
{
    NS_LOG_FUNCTION(this << recipient << +tid);
    // If agreement is still pending, ADDBA response is not received
    if (m_baManager->ExistsAgreementInState(recipient, tid, OriginatorBlockAckAgreement::PENDING))
    {
        m_baManager->NotifyAgreementNoReply(recipient, tid);
        Simulator::Schedule(m_failedAddBaTimeout, &QosTxop::ResetBa, this, recipient, tid);
        GenerateBackoff(SINGLE_LINK_OP_ID);
        if (HasFramesToTransmit(SINGLE_LINK_OP_ID) &&
            GetLink(SINGLE_LINK_OP_ID).access == NOT_REQUESTED)
        {
            m_mac->GetChannelAccessManager(SINGLE_LINK_OP_ID)->RequestAccess(this);
        }
    }
}

void
QosTxop::ResetBa(Mac48Address recipient, uint8_t tid)
{
    NS_LOG_FUNCTION(this << recipient << +tid);
    // This function is scheduled when waiting for an ADDBA response. However,
    // before this function is called, a DELBA request may arrive, which causes
    // the agreement to be deleted. Hence, check if an agreement exists before
    // notifying that the agreement has to be reset.
    if (m_baManager->ExistsAgreement(recipient, tid) &&
        !m_baManager->ExistsAgreementInState(recipient,
                                             tid,
                                             OriginatorBlockAckAgreement::ESTABLISHED))
    {
        m_baManager->NotifyAgreementReset(recipient, tid);
    }
}

void
QosTxop::SetAddBaResponseTimeout(Time addBaResponseTimeout)
{
    NS_LOG_FUNCTION(this << addBaResponseTimeout);
    m_addBaResponseTimeout = addBaResponseTimeout;
}

Time
QosTxop::GetAddBaResponseTimeout() const
{
    return m_addBaResponseTimeout;
}

void
QosTxop::SetFailedAddBaTimeout(Time failedAddBaTimeout)
{
    NS_LOG_FUNCTION(this << failedAddBaTimeout);
    m_failedAddBaTimeout = failedAddBaTimeout;
}

Time
QosTxop::GetFailedAddBaTimeout() const
{
    return m_failedAddBaTimeout;
}

bool
QosTxop::IsQosTxop() const
{
    return true;
}

AcIndex
QosTxop::GetAccessCategory() const
{
    return m_ac;
}

} // namespace ns3

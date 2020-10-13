#ifndef UAVPACKETHEADER_H
#define UAVPACKETHEADER_H

#include "ns3/header.h"

using namespace ns3;

struct Clusters
{
    int ClusterHeadId;
    int ClusterMembersId;
    int DClusterHeadId;
    int TwoHopMember;
    int IntermediateMember;
    int isSecondHop;
    int isDClusterHead;
    int isJoinedCluster;
};
std::vector<Clusters> CH;

Time CurrentTime;
int HeaderType = 0;

class MyHeader : public Header 
 {
 public:
 
   MyHeader ();
   virtual ~MyHeader ();
 
   void SetTimeStamp (Time timeStamp);
   Time GetTimeStamp (void) const;
   void SetResidualEnergy (double residualEnergy);
   double GetResidualEnergy (void) const;
   void SetReputation (double reputation);
   double GetReputation (void) const;
   void SetSpeed (double speed);
   double GetSpeed (void) const;
   void SetNodeId (int nodeId);
   int GetNodeId (void) const;
   void SetMessageType (int messageId);
   int GetMessageType (void) const;
   void SetClusterMembers (std::vector<Clusters>  ClusterMembers);
   std::vector<Clusters>  GetClusterMembers (void) const;

   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual void Print (std::ostream &os) const;
   virtual void Serialize (Buffer::Iterator start) const;
   virtual uint32_t Deserialize (Buffer::Iterator start);
   virtual uint32_t GetSerializedSize (void) const;
 private:
   uint64_t m_timestamp;
   double m_residualenergy;
   double m_reputation ;
   double m_speed;
   int m_nodeid; 
   int m_messagetype; 
 };
 MyHeader::MyHeader ()
 {
 }
 MyHeader::~MyHeader ()
 {
 }
 
 TypeId
 MyHeader::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::MyHeader")
     .SetParent<Header> ()
     .AddConstructor<MyHeader> ()
   ;
   return tid;
 }
 TypeId
 MyHeader::GetInstanceTypeId (void) const
 {
   return GetTypeId ();
 }
 
 void
 MyHeader::Print (std::ostream &os) const
 {
   os << "data=" << m_timestamp;
 }
 uint32_t
 MyHeader::GetSerializedSize (void) const
 {
   return sizeof(CurrentTime)+8+8+8+4+4;
 }
 void
 MyHeader::Serialize (Buffer::Iterator start) const
 {
    Buffer::Iterator i = start;
    i.WriteHtonU64(m_timestamp);
    i.WriteHtonU64(m_residualenergy);
    i.WriteHtonU64(m_reputation);
    i.WriteHtonU64(m_speed);
    i.WriteHtonU32(m_nodeid);
    i.WriteHtonU32(m_messagetype);
 }
 uint32_t
 MyHeader::Deserialize (Buffer::Iterator start)
 {
   Buffer::Iterator i = start;
   m_timestamp = i.ReadNtohU64 ();
   m_residualenergy = i.ReadNtohU64 ();
   m_reputation = i.ReadNtohU64 ();
   m_speed = i.ReadNtohU64 ();
   m_nodeid = i.ReadNtohU32 ();
   m_messagetype = i.ReadNtohU32 ();
   return GetSerializedSize();
 }
 
 void 
 MyHeader::SetTimeStamp (Time timeStamp)
 {
   m_timestamp = timeStamp.GetTimeStep();
 }
 Time 
 MyHeader::GetTimeStamp (void) const
 {
    return TimeStep(m_timestamp);
 }
 void 
 MyHeader::SetResidualEnergy (double residualEnergy)
 {
   m_residualenergy = residualEnergy;
 }
 double 
 MyHeader::GetResidualEnergy (void) const
 {
   return m_residualenergy;
 }
 void 
 MyHeader::SetReputation (double reputation)
 {

   m_reputation = reputation;

 }
 double 
 MyHeader::GetReputation (void) const
 {
   return m_reputation;
 }
 void 
 MyHeader::SetSpeed (double speed)
 {
   m_speed = speed;
 }
 double 
 MyHeader::GetSpeed (void) const
 {
   return m_speed;
 }
 void 
 MyHeader::SetNodeId (int nodeId)
 {
   m_nodeid = nodeId;
 }
 int
 MyHeader::GetNodeId (void) const
 {
   return m_nodeid;
 }
  void 
 MyHeader::SetMessageType (int messageId)
 {
   m_messagetype = messageId;
 }
 int
 MyHeader::GetMessageType (void) const
 {
   return m_messagetype;
 }

class MessageHeader : public Header 
 {
 public:
 
   MessageHeader ();
   virtual ~MessageHeader ();
   void SetTimeStamp (Time timeStamp);
   Time GetTimeStamp (void) const;
   void SetSenderNodeId (int SenderNodeId);
   int  GetSenderNodeId (void) const;
   void SetPacketId (int PacketId);
   int  GetPacketId (void) const;
   void SetClusterHeadId (int ClusterHeadId);
   int  GetClusterHeadId (void) const;
   void SetMessageType (int MessageTypeId);
   int  GetMessageType (void) const;
   void SetSourceAddress (int SrcAddress);
   int  GetSourceAddress (void) const;
   void SetDestinationAddress (int DstAddress);
   int  GetDestinationAddress (void) const;
   void SetIsEdgePacket (int IsEdge);
   int  GetIsEdgePacket (void) const;
   //void SetClusterMembers (std::vector<Clusters>  ClusterMembers);
   //std::vector<Clusters>  GetClusterMembers (void) const;

   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual void Print (std::ostream &os) const;
   virtual void Serialize (Buffer::Iterator start) const;
   virtual uint32_t Deserialize (Buffer::Iterator start);
   virtual uint32_t GetSerializedSize (void) const;
   //virtual void Serialize (Buffer::Iterator start) const;
   //virtual uint32_t Deserialize (Buffer::Iterator start);
   //virtual uint32_t GetSerializedSize (void) const;
 private:
   //std::vector<Clusters> m_clustermembers; 
   uint64_t m_timestamp;
   int m_packetid;
   int m_clusterheadid;
   int m_messagetypeid;
   int m_sourceaddress;
   int m_destinationaddress;
   int m_isedge;
   int m_sendernodeid;
 };
 MessageHeader::MessageHeader ()
 {
 }
 MessageHeader::~MessageHeader ()
 {
 }
TypeId
 MessageHeader::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::MessageHeader")
     .SetParent<Header> ()
     .AddConstructor<MessageHeader> ()
   ;
   return tid;
 }
 TypeId
 MessageHeader::GetInstanceTypeId (void) const
 {
   return GetTypeId ();
 }
 
 void
 MessageHeader::Print (std::ostream &os) const
 {
   //test
 }

   uint32_t
 MessageHeader::GetSerializedSize (void) const
 {
   return 4+4+4+4+4+4+4+sizeof(CurrentTime);
 }
 void
 MessageHeader::Serialize (Buffer::Iterator start) const
 {
    Buffer::Iterator i = start;
    i.WriteHtonU64(m_timestamp);
    i.WriteHtonU32(m_sendernodeid);
    i.WriteHtonU32(m_packetid);
    i.WriteHtonU32(m_clusterheadid);
    i.WriteHtonU32(m_messagetypeid);
    i.WriteHtonU32(m_sourceaddress);
    i.WriteHtonU32(m_destinationaddress);
    i.WriteHtonU32(m_isedge);
 }
 uint32_t
 MessageHeader::Deserialize (Buffer::Iterator start)
 {
   Buffer::Iterator i = start;
   m_timestamp = i.ReadNtohU64 ();
   m_sendernodeid = i.ReadNtohU32();
   m_packetid = i.ReadNtohU32 ();
   m_clusterheadid = i.ReadNtohU32 ();
   m_messagetypeid = i.ReadNtohU32 ();
   m_sourceaddress = i.ReadNtohU32 ();
   m_destinationaddress = i.ReadNtohU32 ();
   m_isedge = i.ReadNtohU32();
   return GetSerializedSize();
 }
void 
MessageHeader::SetTimeStamp (Time timeStamp)
{
  m_timestamp = timeStamp.GetTimeStep();
}
Time 
MessageHeader::GetTimeStamp (void) const
{
  return TimeStep(m_timestamp);
}
void 
 MessageHeader::SetSenderNodeId (int SenderNodeId)
 {
    m_sendernodeid = SenderNodeId;
 }
 int
 MessageHeader::GetSenderNodeId (void) const
 {
   return m_sendernodeid;
 }

 void 
 MessageHeader::SetPacketId (int PacketId)
 {
    m_packetid = PacketId;
 }
 int
 MessageHeader::GetPacketId (void) const
 {
   return m_packetid;
 }
 
 void 
 MessageHeader::SetClusterHeadId (int ClusterHeadId)
 {
    m_clusterheadid = ClusterHeadId;
 }
 int
 MessageHeader::GetClusterHeadId (void) const
 {
   return m_clusterheadid;
 }

  void 
  MessageHeader::SetMessageType (int MessageTypeId)
  {
    m_messagetypeid = MessageTypeId;
  }
  int   
  MessageHeader::GetMessageType (void) const
  {
    return m_messagetypeid;
  }
  void 
  MessageHeader::SetSourceAddress (int SrcAddress)
  {
    m_sourceaddress = SrcAddress;
  }
  int   
  MessageHeader::GetSourceAddress (void) const
  {
    return m_sourceaddress;
  }
  void  
  MessageHeader::SetDestinationAddress (int DstAddress)
  {
    m_destinationaddress = DstAddress;
  }
  int   
  MessageHeader::GetDestinationAddress (void) const
  {
    return m_destinationaddress;
  }
  void 
  MessageHeader::SetIsEdgePacket (int IsEdge)
  {
    m_isedge = IsEdge;
  }
  int   
  MessageHeader::GetIsEdgePacket (void) const
  {
    return m_isedge;
  }

 /*
 uint32_t
 MessageHeader::GetSerializedSize (void) const
 {
   return (m_clustermembers.size()*4)*8+4;
 }
 void
 MessageHeader::Serialize (Buffer::Iterator start) const
 {
    Buffer::Iterator i = start;
    i.WriteHtonU32(m_clustermembers.size());
    for(std::vector<int>::size_type k = 0; k < m_clustermembers.size(); k++)
    {
      i.WriteHtonU32(m_clustermembers[k].ClusterHeadId);
      i.WriteHtonU32(m_clustermembers[k].ClusterMembersId);
      i.WriteHtonU32(m_clustermembers[k].DClusterHeadId);
      i.WriteHtonU32(m_clustermembers[k].IntermediateMember);
      i.WriteHtonU32(m_clustermembers[k].isDClusterHead);
      i.WriteHtonU32(m_clustermembers[k].isJoinedCluster);
      i.WriteHtonU32(m_clustermembers[k].isSecondHop);
      i.WriteHtonU32(m_clustermembers[k].TwoHopMember);
    }
 }
 uint32_t
 MessageHeader::Deserialize (Buffer::Iterator start)
 {
   Buffer::Iterator i = start;
   uint32_t len1 = i.ReadNtohU32 ();
   for(uint32_t k = 0; k < len1; k++)
   { 
      Clusters tempc;
      tempc.ClusterHeadId = i.ReadNtohU32();
      tempc.ClusterMembersId = i.ReadNtohU32();
      tempc.DClusterHeadId = i.ReadNtohU32();
      tempc.IntermediateMember = i.ReadNtohU32();
      tempc.isDClusterHead = i.ReadNtohU32();
      tempc.isJoinedCluster = i.ReadNtohU32();
      tempc.isSecondHop = i.ReadNtohU32();
      tempc.TwoHopMember = i.ReadNtohU32();
      m_clustermembers.push_back(tempc);
   }
   return GetSerializedSize();
 }
 void 
MessageHeader::SetClusterMembers (std::vector<Clusters> ClusterMembers)
{
  m_clustermembers = ClusterMembers;
}
std::vector<Clusters>
MessageHeader::GetClusterMembers (void) const
{
  return m_clustermembers;
}
*/

#endif
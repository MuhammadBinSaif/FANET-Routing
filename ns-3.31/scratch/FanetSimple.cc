#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "ns3/random-variable-stream.h"
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/filters.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include "crypto++/cryptlib.h"
#include "crypto++/sha.h"
#include "crypto++/hex.h"
#include "uavPopulation.h"
#include <vector>
#include "uavPacketHeader.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Fanet");

// Functions Declaration

double 
CalculateReputation(int Rx, int Tx);

void
UpdateReputation();

inline 
KeyPairHex RsaGenerateHexKeyPair(unsigned int aKeySize);

void
GenerateKeyPair();

inline 
std::string RsaSignString(const std::string &aPrivateKeyStrHex, const std::string &aPublicKey);

void
GenerateTransactions(int chId, int recieverId);

std::string 
hex_to_string(const std::string& input);

std::string 
string_to_hex(const std::string& input);

std::string 
shaa256(std::string msg);

std::string
GenerateTransactionHash(Transaaction Transaction);

std::string
GenerateBlockHash(std::vector<Transaaction> Transaactions);

void 
GenerateStatsCsv();

void 
ReadStatsCsv();

void 
BlankOutCsvFile();

static inline std::string 
GetStringNodeId (Ptr<Node> node);

Ptr<Node>	
GetNodeFromAddress (Ipv4Address ipAddress);

Ipv4Address	
GetNodeAddress (int nodeId);

MessageHeader
CastHeader(MessageHeader header);

void 
HandlePacket (Ptr<Socket> socket);

void 
HandleHelloPacket (Ptr<Socket> socket);

void 
HandleClusterHeadAnnouncementPacket (Ptr<Socket> socket);

void 
HandleTrafficPacket (Ptr<Socket> socket);

static void 
GenerateHelloBroadcast(Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval);

static void 
GenerateHelloCluster(Ptr<Socket> socket, uint32_t packetSize, uint32_t packetCount, Time packetInterval);

static void 
SendClusterJoinRequest();

static void 
SendTaraficPackets();

void 
SendPacket(Ptr<Packet> packet, int nextHop);

void 
SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);

EdgeNextHop 
FindEdgeNode (int Source,int Destination);

int
FindClusterHead(int SourceNode);

NextHop
FindNextHop(int SourceNode, int DestinationNode);

int
FindIntermediateHop(int SourceNode);

void 
CheckThroughput ();

std::vector<Neighbours>
CheckorAddNeighbours(int recieverId , int senderId, std::vector<Neighbours> nodeIdList);

void
CheckorAddTwoHopClusterMembers();

double
GetVelocityCourseChange(int nodeId);

double 
GetNodeRemainingEnergy(int nodeId);

void 
InitializeGlobalValues();

void 
InstallEnergyModel(NodeContainer & uavNodes, NetDeviceContainer & UAVadhocDevices);

void 
InstallMobilityModel(NodeContainer & uavNodes);

void 
CourseChange (std::string context, Ptr<const MobilityModel> model);

// Cluster Head Selection EABC 

static void 
ClusterHeadSelection ();

void InitPopulation();

void init();

double calculate_function(double solution[PARAMS_SIZE]);

double calculate_fitness(double value);

double get_random_number();

void init_bee(int index);

void send_employed_bees();

void calculate_probabilities();
    
void send_onlooker_bees();

void send_scout_bees();

void get_gbest();


// Main Function Section

int
main (int argc, char *argv[])
{
  InitializeGlobalValues();
  BlankOutCsvFile();
  int UAVnodes = gNodes;

  NodeContainer UAVadhocNodes;
  UAVadhocNodes.Create (UAVnodes);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("ErpOfdmRate54Mbps"));

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  //Config::SetDefault( "ns3::RangePropagationLossModel::MaxRange", DoubleValue (maxRange));
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();  
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  //wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (maxRange));
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set ("TxPowerStart",DoubleValue (300));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (300));
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
 
  NetDeviceContainer UAVadhocDevices = wifi.Install (wifiPhy, wifiMac, UAVadhocNodes);

  InstallEnergyModel(UAVadhocNodes, UAVadhocDevices);
  
  InstallMobilityModel(UAVadhocNodes);

  InternetStackHelper internet;
  internet.Install (UAVadhocNodes);

  NS_LOG_INFO ("assigning ip address");
  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign (UAVadhocDevices);

//ReadStatsCsv();

  uint32_t nNodes = UAVadhocNodes.GetN ();
  for (uint32_t n = 0; n < nNodes; ++n)
    {
      Ptr<Node> p = UAVadhocNodes.Get(n);
      int NodeId = p->GetId();
      RR[n].NodeId = NodeId;
      RR[n].Reputation = 0;
      RR[n].RxPackets = 0;
      RR[n].TxPackets = 0;
      RR[n].DrpPackets = 0;
      RR[n].SimulationTime = Simulator::Now().GetSeconds();
      RR[n].IpAddress = GetNodeAddress(NodeId);
      UP[n].NodeId = NodeId;
      UP[n].IpAddress = GetNodeAddress(NodeId);
      UP[n].RemainingEnergy = UAVEnergy;
      UP[n].ReputationRanking = RR[n].Reputation;
      UP[n].TotalOnlineTime = 0;
      UP[n].Transactions = 0;
      UP[n].Latency = 0;
      UP[n].Speed = 50;
      UP[n].CourseChangeCount = 0;
      UP[n].SimulationTime = Simulator::Now().GetSeconds();
    }

  GenerateKeyPair();
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
   
  static const InetSocketAddress Broadcast = InetSocketAddress(Ipv4Address("255.255.255.255"), port);
  
  for (uint32_t j=0; j<nNodes; j++)
  {
    Ptr<Socket> recvSink = Socket::CreateSocket (UAVadhocNodes.Get (j), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&HandlePacket));
 
    Ptr<Socket> source = Socket::CreateSocket (UAVadhocNodes.Get (j), tid);
    source->SetAllowBroadcast (true);
    source->Connect (Broadcast);
    HelloTimer =1.0+ ((double) rand() / (RAND_MAX));
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (), Seconds(HelloTimer), &GenerateHelloBroadcast, source, packetSize, packetCount, packetInterval);
    
  } 
  ClusterHeadSelectionTimer = HelloTimer+1;
  ClusterHeadSelectionEventId = Simulator::Schedule(Seconds(ClusterHeadSelectionTimer), &ClusterHeadSelection);
  TwoHopTimer = ClusterHeadSelectionTimer+0.79;
  Simulator::Schedule(Seconds(TwoHopTimer), &CheckorAddTwoHopClusterMembers);
  TrafficTimer = ClusterHeadSelectionTimer+0.8;
  TrafficEventId = Simulator::Schedule(Seconds(TrafficTimer), &SendTaraficPackets);

  NS_LOG_INFO ("Run Simulation.");

  CheckThroughput ();
  
  Simulator::Stop (Seconds (TotalTime));

  AnimationInterface anim("FanetSimple.xml");
  anim.EnablePacketMetadata(true);
  anim.UpdateNodeColor(1,255,255,0);
  anim.UpdateNodeColor(3,255,255,0);
  anim.UpdateNodeColor(9,255,255,0);
  anim.UpdateNodeColor(19,255,255,0);
  anim.UpdateNodeColor(14,255,255,0);
  anim.UpdateNodeColor(19,255,255,0);

  Simulator::Run ();

  GenerateStatsCsv();

  Simulator::Destroy ();

  return 0;
}

// Csv Generator and Reader Functions Section

double
CalculateReputation(int Rx, int Tx)
{
 return (Rx+Tx)/gNodes;
}

void
UpdateReputation()
{
  for(int n=0; n<gNodes; n++){
    double updatedReputation = CalculateReputation(RR[n].RxPackets,RR[n].TxPackets);
    RR[n].Reputation = updatedReputation;
  }
}

inline KeyPairHex RsaGenerateHexKeyPair(unsigned int aKeySize) {
  KeyPairHex keyPair;
  // PGP Random Pool-like generator
  CryptoPP::AutoSeededRandomPool rng;
  // generate keys
  CryptoPP::RSA::PrivateKey privateKey;
  privateKey.GenerateRandomWithKeySize(rng, aKeySize);
  CryptoPP::RSA::PublicKey publicKey(privateKey);
  // save keys
  publicKey.Save( CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(keyPair.publicKey)).Ref());
  privateKey.Save(CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(keyPair.privateKey)).Ref());
  return keyPair;
}

void
GenerateKeyPair(){
  for(int n=0; n<gNodes; n++){
    auto keys = RsaGenerateHexKeyPair(3072);
    KeyPair KeyPair;
    KeyPair.NodeId = n;
    KeyPair.PrivateKey = keys.privateKey;
    KeyPair.PublicKey = keys.publicKey;
    KeyPairs.push_back(KeyPair);
  }
}

inline std::string RsaSignString(const std::string &aPrivateKeyStrHex,
                                 const std::string &aPublicKey) {

  // decode and load private key (using pipeline)
  CryptoPP::RSA::PrivateKey privateKey;
  privateKey.Load(CryptoPP::StringSource(aPrivateKeyStrHex, true,
                                         new CryptoPP::HexDecoder()).Ref());

  // sign message
  std::string signature;
  Signer signer(privateKey);
  CryptoPP::AutoSeededRandomPool rng;

  CryptoPP::StringSource ss(aPublicKey, true,
                            new CryptoPP::SignerFilter(rng, signer,
                              new CryptoPP::HexEncoder(
                                new CryptoPP::StringSink(signature))));

  return signature;
}

std::string
GenerateTransactionHash(Transaaction Transaction){
  std::string TransactionString = Transaction.Reputation+","+Transaction.TimeStamp+","+Transaction.TransactionMetadata.CourseChangeCount+","+Transaction.TransactionMetadata.Latency+","+Transaction.TransactionMetadata.RemainingEnergy+","+Transaction.TransactionMetadata.Speed+","+Transaction.TransactionMetadata.TotalOnlineTime+","+Transaction.TransactionMetadata.Transactions;
  std::string TransactionHash = shaa256(TransactionString);
  return string_to_hex(TransactionHash);
}

std::string
GenerateBlockHash(std::vector<Transaaction> Transaactions){
  std::string TransactionsHash;
  for(std::vector<int>::size_type i = 0; i != Transaactions.size(); i++){
    TransactionsHash = TransactionsHash+" "+Transaactions[i].Hash; 
  }
  td::string BlockHash = shaa256(TransactionsHash);
  return string_to_hex(BlockHash);
}

std::string 
hex_to_string(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();
    if (len & 1) throw std::invalid_argument("odd length");
    std::string output;
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");
        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");
        output.push_back(((p - lut) << 4) | (q - lut));
    }
    return output;
}

std::string 
string_to_hex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();
    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

std::string 
shaa256(std::string msg)
{
  HexEncoder encoder(new FileSink(std::cout));
  std::string digest;
  SHA256 hash;
  hash.Update((const byte*)msg.data(), msg.size());
  digest.resize(hash.DigestSize());
  hash.Final((byte*)&digest[0]);
  //std::cout << "Message:" << msg << std::endl;
  string s=string_to_hex(digest);
  //std::cout << "Digest: "<<s<<endl;
  //uint32_t ff=atoi(s.c_str());
 //StringSource(digest, true, new Redirector(encoder));
  //cout<<"cc"<<digest<<endl;
  return s;
}

void
GenerateTransactions(int chId, int recieverId){
  
  Transaaction SingleTransaction;
  TransactionCount++;
  SingleTransaction.TransactionId = TransactionCount;
  SingleTransaction.PreviousTransactionId = TransactionCount-1;
  SingleTransaction.Reputation = UP[recieverId].ReputationRanking;
  SingleTransaction.TransactionMetadata = UP[recieverId];
  SingleTransaction.PublicKey = KeyPairs[chId].PublicKey;
  SingleTransaction.TimeStamp = Simulator::Now();
  SingleTransaction.Signature = auto signature(RsaSignString(KeyPairs[recieverId].PrivateKey, KeyPairs[recieverId].PublicKey));
  SingleTransaction.NextPublicKey = KeyPairs[recieverId].PublicKey;
  GenerateTransactionHash(SingleTransaction);
  SingleTransaction.Hash=Transaactions.push_back(SingleTransaction);
  Transaactions.push_back(SingleTransaction);
  if(Transaactions.size() == 5){
    BlockCount++;
    Block SingleBlock;
    int witness = rand() % 79 + 0;
    SingleBlock.WitnessSignature = auto signature(RsaSignString(KeyPairs[witness].PrivateKey, KeyPairs[witness].PublicKey));
    SingleBlock.Hash = GenerateBlockHash(Transaactions);
    SingleBlock.PreviousHash = Blocks[BlockCount-1].Hash;
    SingleBlock.TimeStamp = Simulator::Now();
    SingleBlock.Transactions = Transaactions;
    Blocks.push_back(SingleBlock);
    Transaactions.clear();
  }
}

void 
GenerateStatsCsv()
{
  std::string m_CSVfileName = "Node-Reputationtemp.csv";
  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);
  for (int n = 0; n < gNodes; ++n)
  {
    out << RR[n].NodeId << ","
    << RR[n].TxPackets << ","
    << RR[n].RxPackets << ","
    << RR[n].IpAddress << ","
    << RR[n].SimulationTime << ","
    << CalculateReputation(RR[n].RxPackets, RR[n].TxPackets) << ""
    << std::endl;
  }
  out.close ();
  remove("Node-Reputation.csv"); 
  rename("Node-Reputationtemp.csv", "Node-Reputation.csv"); 

  std::string m_CSVfileName1 = "Node-Statstemp.csv";
  std::ofstream out1 (m_CSVfileName1.c_str (), std::ios::app);
  for (int n = 0; n < gNodes; ++n)
  {
    out1 << UP[n].NodeId << ","
    << UP[n].IpAddress << ","
    << UP[n].RemainingEnergy << ","
    << UP[n].ReputationRanking << ","
    << UP[n].TotalOnlineTime << ","
    << UP[n].Transactions << ","
    << UP[n].Speed << ","
    << UP[n].CourseChangeCount << ","
    << UP[n].Latency << ","
    << UP[n].SimulationTime << ""
    << std::endl;
  }
  out1.close ();
  remove("Node-Stats.csv"); 
  rename("Node-Statstemp.csv", "Node-Stats.csv"); 

}

void 
ReadStatsCsv()
{
  std::ifstream RRFile("Node-Reputation.csv");
  if(!RRFile.is_open()) throw std::runtime_error("Could not open file");
  std::string line;
  int lineCount = 0;
  while (std::getline(RRFile, line) && !line.empty())
  {
    std::stringstream mystream(line);
    std::string Nodeid, Rx, Tx, Reputation, ipv4, simtime;
    std::getline(mystream, Nodeid, ',');
    std::getline(mystream, Tx, ',');
    std::getline(mystream, Rx, ',');
    std::getline(mystream, ipv4, ',');
    std::getline(mystream, simtime, ',');
    std::getline(mystream, Reputation, ',');
    RR[lineCount].NodeId = stoi(Nodeid);
    RR[lineCount].TxPackets = stoi(Tx);
    RR[lineCount].RxPackets = stoi(Rx);
    RR[lineCount].IpAddress = GetNodeAddress(RR[lineCount].NodeId);
    RR[lineCount].SimulationTime = stod(simtime);
    RR[lineCount].Reputation = stod(Reputation);
    if (!mystream)
        break; // something went wrong reading the line
    lineCount++;
  }
  RRFile.close();

  std::ifstream SRFile("Node-Stats.csv");
  if(!SRFile.is_open()) throw std::runtime_error("Could not open file");
  std::string line1;
  int lineCount1 = 0;
  while (std::getline(SRFile, line1) && !line1.empty())
  {
    std::stringstream mystream(line1);
    std::string Nodeid, IpAddress, RemainingEnergy, ReputationRanking, TotalOnlineTime, Transactions, Speed, CourseChangeCount, Latency, SimulationTime;
    std::getline(mystream, Nodeid, ',');
    std::getline(mystream, IpAddress, ',');
    std::getline(mystream, RemainingEnergy, ',');
    std::getline(mystream, ReputationRanking, ',');
    std::getline(mystream, TotalOnlineTime, ',');
    std::getline(mystream, Transactions, ',');
    std::getline(mystream, Speed, ',');
    std::getline(mystream, CourseChangeCount, ',');
    std::getline(mystream, Latency, ',');
    std::getline(mystream, SimulationTime, ',');
    UP[lineCount1].NodeId = stoi(Nodeid);
    UP[lineCount1].IpAddress = GetNodeAddress(UP[lineCount1].NodeId);
    UP[lineCount1].RemainingEnergy = 0;
    UP[lineCount1].ReputationRanking = stod(ReputationRanking);
    UP[lineCount1].TotalOnlineTime = stod(TotalOnlineTime);
    UP[lineCount1].Transactions = stoi(Transactions);
    UP[lineCount1].Speed = 0;
    UP[lineCount1].CourseChangeCount = 0;
    UP[lineCount1].Latency = 0;
    UP[lineCount1].SimulationTime = stod(SimulationTime);

    if (!mystream)
        break; // something went wrong reading the line
    lineCount1++;
  }
  SRFile.close();
}

void 
BlankOutCsvFile()
{
  std::ofstream out (m_CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  "TransmissionPower" <<
  std::endl;
  out.close ();
}

//Address Converter Functions Section

static inline std::string
GetStringNodeId (Ptr<Node> node)
{
  uint32_t id = node->GetId();
  std::ostringstream oss;
  oss <<id;
  return oss.str ();
}

Ptr<Node>	
GetNodeFromAddress (Ipv4Address ipAddress)
{
  int32_t nNodes = NodeList::GetNNodes ();
  for (int32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      int32_t ifIndex = ipv4->GetInterfaceForAddress (ipAddress);
      if (ifIndex != -1)
        {
          return node;
        }
    }
  return 0;
}

Ipv4Address	
GetNodeAddress (int nodeId)
{
  Ptr<Node> node = NodeList::GetNode (nodeId);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address ifIndex = ipv4->GetAddress (1, 0).GetLocal ();
  return	ifIndex;
}

MessageHeader
CastHeader(MessageHeader header)
{
  MessageHeader newheader;
  newheader.SetClusterHeadId(header.GetClusterHeadId());
  newheader.SetDestinationAddress(header.GetDestinationAddress());
  newheader.SetIsEdgePacket(header.GetIsEdgePacket());
  newheader.SetMessageType(header.GetMessageType());
  newheader.SetSourceAddress(header.GetSourceAddress());
  newheader.SetTimeStamp(header.GetTimeStamp());
  return newheader;
}

//Trafic Generator and Reciever Functions Section


void HandlePacket (Ptr<Socket> socket)
{
  if(HeaderType == 0){
    // handle hello broadcast

    HandleHelloPacket(socket);
  }
  else if(HeaderType == 1){
    // Handle Hello cluster join/Announcement

    HandleClusterHeadAnnouncementPacket(socket);
  }
  else if(HeaderType == 2){
    // Handle normal traffic
    
    HandleTrafficPacket(socket);
  }

}

void 
HandleHelloPacket (Ptr<Socket> socket)
{
  int recieverNodeId;
  int senderNodeId;
  Ptr<Packet> packet;
  Address senderAd;
  Ptr<Node> receiver = socket->GetNode ();
  recieverNodeId = receiver->GetId();
  while (packet = socket->RecvFrom (senderAd))
  {
    MyHeader destinationHeader;
    packet->RemoveHeader (destinationHeader);
    int nodeId = destinationHeader.GetNodeId();
    senderNodeId = nodeId;
    
    //Update Residual Energy of node
    UP[nodeId].RemainingEnergy = destinationHeader.GetResidualEnergy();

    //Set Upper bound of residual energy
    if(UBRemainingEnergy < UP[nodeId].RemainingEnergy)
    {
      UBRemainingEnergy = UP[nodeId].RemainingEnergy;
      bounds_matrix[0][1] = UBRemainingEnergy;
    }

    //Update Reputation of Sender Node
    if(RR[nodeId].Reputation == destinationHeader.GetReputation())
    {
      RR[receiver->GetId()].RxPackets += 1;
      UpdateReputation();
    }
    else{
      RR[nodeId].Reputation = 0;
      RR[receiver->GetId()].RxPackets += 1;
      UpdateReputation();
    } 

    //Set upper bound of reputation
    UP[nodeId].ReputationRanking = RR[nodeId].Reputation;
    if(UBReputationRanking < UP[nodeId].ReputationRanking)
    {
      UBReputationRanking = UP[nodeId].ReputationRanking;
      bounds_matrix[1][1] = UBReputationRanking;
    }

    //Update Online time of node
    UP[nodeId].TotalOnlineTime += Simulator::Now().GetSeconds();

    //Set upper bound of Online Time
    if(UBTotalOnlineTime < UP[nodeId].TotalOnlineTime)
    {
      UBTotalOnlineTime = UP[nodeId].TotalOnlineTime;
      bounds_matrix[2][1] = UBTotalOnlineTime;
    }

    //Update latency
    double latency = Simulator::Now().GetSeconds()- destinationHeader.GetTimeStamp().GetSeconds();
    UP[nodeId].Latency += latency;
    
    // Set upper boud of latency
    if(UBLatency < UP[nodeId].Latency)
    {
      UBLatency = UP[nodeId].Latency;
      bounds_matrix[4][1] = UBLatency;
    }

    //Update Speed of node
    UP[nodeId].Speed = destinationHeader.GetSpeed();

    //Set Upper bound of speed
    if(UBSpeed < UP[nodeId].Speed)
    {
      UBSpeed = UP[nodeId].Speed;
      bounds_matrix[5][1] = UBSpeed;
    }
  }
  std::cout<<recieverNodeId<<" Recieved hello packet from "<<senderNodeId<<" time is "<<Simulator::Now().GetSeconds()<<std::endl;
  //Update Neighbours of Nodes
  NodeIds = CheckorAddNeighbours(recieverNodeId,senderNodeId,NodeIds);
}

void 
HandleClusterHeadAnnouncementPacket (Ptr<Socket> socket)
{
  int recieverNodeId;
  int senderNodeId;
  Ptr<Packet> packet;
  Address senderAd;
  Ptr<Node> receiver = socket->GetNode ();
  recieverNodeId = receiver->GetId();
  while (packet = socket->RecvFrom (senderAd))
  {
    MessageHeader destinationHeader;
    packet->RemoveHeader (destinationHeader);
    if(destinationHeader.GetMessageType() == 1)
    {
      int  tempch;
      bool foundmember = false;
      bool founddch = false;
      tempch = destinationHeader.GetClusterHeadId();
      senderNodeId = tempch;
      if(CH.size()== 0){

        Clusters tempc;
        for(int ch=0; ch<16; ch++){
          if(clusterHeadId[ch] == recieverNodeId)
          {
            founddch = true;
            break;
          }
          else
          {
            founddch = false;
          }
        }
        if(founddch == true){
          tempc.ClusterHeadId = tempch;
          tempc.ClusterMembersId = -1;
          tempc.DClusterHeadId = recieverNodeId;
          tempc.IntermediateMember = -1;
          tempc.isDClusterHead = 1;
          tempc.isJoinedCluster = 1;
          tempc.isSecondHop = 0;
          tempc.TwoHopMember = -1;
          GenerateTransactions(tempch, recieverNodeId);
          CH.push_back(tempc);
        }
        else{
          tempc.ClusterHeadId = tempch;
          tempc.ClusterMembersId = recieverNodeId;
          tempc.DClusterHeadId = -1;
          tempc.IntermediateMember = -1;
          tempc.isDClusterHead = 0;
          tempc.isJoinedCluster = 1;
          tempc.isSecondHop = 0;
          tempc.TwoHopMember = -1;
          GenerateTransactions(tempch, recieverNodeId);
          CH.push_back(tempc);
        }
      }
      else{
        for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
        {
          if(CH[i].ClusterMembersId == recieverNodeId){
              foundmember = true;
              EdgeNode tempe;
              tempe.EdgeNodeId = recieverNodeId;
              tempe.FirstClusterHeadId = CH[i].ClusterHeadId;
              tempe.SecondClusterHeadId = tempch;
              EdgeNodes.push_back(tempe);
              break;
            }
            else{
              foundmember = false;
            }
        }
        if(foundmember == false)
        {
          Clusters tempc;
          for(int ch=0; ch<16; ch++){
            if(clusterHeadId[ch] == recieverNodeId)
            {
              founddch = true;
              break;
            }
            else
            {
              founddch = false;
            }
          }
          if(founddch == true){
            tempc.ClusterHeadId = tempch;
            tempc.ClusterMembersId = -1;
            tempc.DClusterHeadId = recieverNodeId;
            tempc.IntermediateMember = -1;
            tempc.isDClusterHead = 1;
            tempc.isJoinedCluster = 1;
            tempc.isSecondHop = 0;
            tempc.TwoHopMember = -1;
            GenerateTransactions(tempch, recieverNodeId);
            CH.push_back(tempc);
          }
          else{
            tempc.ClusterHeadId = tempch;
            tempc.ClusterMembersId = recieverNodeId;
            tempc.DClusterHeadId = -1;
            tempc.IntermediateMember = -1;
            tempc.isDClusterHead = 0;
            tempc.isJoinedCluster = 1;
            tempc.isSecondHop = 0;
            tempc.TwoHopMember = -1;
            GenerateTransactions(tempch, recieverNodeId);
            CH.push_back(tempc);
          }
        }
      }
    }
  }

  std::cout<<recieverNodeId<<" Recieved hello Cluster Head packet from "<<senderNodeId<<" time is "<<CH.size()<<std::endl;
}

void 
HandleTrafficPacket (Ptr<Socket> socket)
{
  std::cout<<"inside "<<Simulator::Now().GetSeconds()<<std::endl;
  int recieverNodeId;
  int senderNodeId;
  Ptr<Packet> packet;
  Address senderAd;
  Ptr<Node> receiver = socket->GetNode ();
  recieverNodeId = receiver->GetId();
  while (packet = socket->RecvFrom (senderAd))
  {
    MessageHeader destinationHeader;
    packet->PeekHeader(destinationHeader);
    MessageHeader newHeader;
    if(destinationHeader.GetMessageType()== 2)
    {
      senderNodeId = destinationHeader.GetSenderNodeId();
      std::cout<<"Node "<<recieverNodeId<<" recieved packet from "<<senderNodeId<<std::endl;
      if(recieverNodeId == destinationHeader.GetDestinationAddress())
      {
        std::cout<<"packet delivered successfully source node= "<<destinationHeader.GetSourceAddress()<<" Destination node is "<<recieverNodeId<<std::endl;
        recievepacketcount++;
        std::cout<<"Total recieved packets "<<recievepacketcount<<std::endl;
        std::cout<<"Total sent packets "<<sendpacketcount<<std::endl;
        double delay = Simulator::Now().GetSeconds()-destinationHeader.GetTimeStamp().GetSeconds();
        etoedelay += delay;
        bytesTotal += packet->GetSize ();
        float PDR = (recievepacketcount)/(sendpacketcount);
        std::cout<<"End to end delay= "<<etoedelay/recievepacketcount<<"Block Count"<<Blocks[30].WitnessSignature<<" PDR= "<<PDR<<" Throughput= "<<(bytesTotal * 8.0) / 1000<<std::endl;
        return;
      }
      else if(recieverNodeId == destinationHeader.GetClusterHeadId())
      {
        NextHop nextHop = FindNextHop(recieverNodeId, destinationHeader.GetDestinationAddress());
        if(nextHop.NextHopType != -1)
        {
          if(nextHop.NextHopType == 0 || nextHop.NextHopType == 1)
          {
            std::cout<<"Cluster head recieved packet sending to direct cluster head or member"<<std::endl;
            newHeader = CastHeader(destinationHeader);
            newHeader.SetIsEdgePacket(0);
            newHeader.SetSenderNodeId(destinationHeader.GetClusterHeadId());
            //Direct Cluster Member
            packet->AddHeader(newHeader);
            SendPacket(packet,nextHop.NextHopId);
          }
          else if(nextHop.NextHopType == 2)
          {
            std::cout<<"Cluster head recieved packet but destination is two hop away"<<std::endl;
            newHeader = CastHeader(destinationHeader);
            newHeader.SetIsEdgePacket(0);
            newHeader.SetClusterHeadId(-1);
            newHeader.SetSenderNodeId(destinationHeader.GetClusterHeadId());
            packet->AddHeader(newHeader);
            // two hop neighbour
            // put cluster head id to -1 for forwarding
            SendPacket(packet,nextHop.NextHopId);
          }
          else if(nextHop.NextHopType == 3)
          {
            EdgeNextHop eh;
            std::cout<<"Cluster head recievd packet but not found in destination finding edge node"<<std::endl;
            NextHop dnextHop = FindNextHop(recieverNodeId, nextHop.NextHopId);
            //if CH is direct connected DCH
            if(dnextHop.NextHopType == 1){
              newHeader = CastHeader(destinationHeader);
              newHeader.SetIsEdgePacket(0);
              newHeader.SetClusterHeadId(dnextHop.NextHopId);
              newHeader.SetSenderNodeId(destinationHeader.GetClusterHeadId());
              packet->AddHeader(newHeader);
              // send  it in other cluster head
              SendPacket(packet,dnextHop.NextHopId);
              return;
            }
            // else find edge node
            else{
              eh = FindEdgeNode(recieverNodeId,nextHop.NextHopId);
              if(eh.EdgeNodeId != -1)
              {
                newHeader = CastHeader(destinationHeader);
                newHeader.SetIsEdgePacket(1);
                //here is issue
                newHeader.SetClusterHeadId(eh.ClusterHeadId);
                newHeader.SetSenderNodeId(destinationHeader.GetClusterHeadId());
                packet->AddHeader(newHeader);
                // send  it in other cluster
                SendPacket(packet,eh.EdgeNodeId);
              }
              else{
                //sendpacketcount--;
                newHeader = CastHeader(destinationHeader);
                packet->AddHeader(newHeader);
                SendPacket(packet,newHeader.GetDestinationAddress());
              }
            }
          }
          else if(nextHop.NextHopType == 4)
          {
            NextHop tnextHop = FindNextHop(recieverNodeId, nextHop.NextHopId);
            if(tnextHop.NextHopType == 1){
              newHeader = CastHeader(destinationHeader);
              newHeader.SetIsEdgePacket(0);
              newHeader.SetSenderNodeId(recieverNodeId);
              newHeader.SetClusterHeadId(tnextHop.NextHopId);
              //Direct Cluster Head

              packet->AddHeader(newHeader);
              SendPacket(packet,tnextHop.NextHopId);
            }
            else
            {
              EdgeNextHop eh = FindEdgeNode(recieverNodeId,nextHop.NextHopId);
              if(eh.EdgeNodeId != -1)
              {
                newHeader = CastHeader(destinationHeader);
                newHeader.SetIsEdgePacket(1);
                newHeader.SetClusterHeadId(eh.ClusterHeadId);
                newHeader.SetSenderNodeId(recieverNodeId);
                packet->AddHeader(newHeader);
                // send  it in other cluster
                SendPacket(packet,eh.EdgeNodeId);
              }
            }
          }
        }
        else
        {
          //sendpacketcount--;
          newHeader = CastHeader(destinationHeader);
          packet->AddHeader(newHeader);
          SendPacket(packet,newHeader.GetDestinationAddress());
        }
      }
      else if(destinationHeader.GetIsEdgePacket() == 1){
        std::cout<<"Edge Node recieved packet forwarding it to cluster head"<<std::endl;
        int CHid = FindClusterHead(recieverNodeId);
        if(CHid == -1){
          return;
        }
        std::cout<<CHid<<std::endl;
        newHeader = CastHeader(destinationHeader);
        newHeader.SetIsEdgePacket(0);
        newHeader.SetClusterHeadId(CHid);
        newHeader.SetSenderNodeId(recieverNodeId);
        packet->AddHeader(newHeader);
        if(destinationHeader.GetClusterHeadId() == -1){
          SendPacket(packet,CHid);
        }
        else{
          SendPacket(packet,destinationHeader.GetClusterHeadId());
        }
      }
      else if(destinationHeader.GetClusterHeadId() == -1 ){
        std::cout<<"Node recieved two hop packet forwarding it to destination"<<std::endl;
        newHeader = CastHeader(destinationHeader);
        newHeader.SetIsEdgePacket(0);
        newHeader.SetClusterHeadId(-3);
        newHeader.SetSenderNodeId(recieverNodeId);
        packet->AddHeader(newHeader);
        SendPacket(packet,destinationHeader.GetDestinationAddress());
      }
      else if(destinationHeader.GetClusterHeadId() == -2 ){
        std::cout<<"Node recieved two hop packet forwarding it to CH"<<std::endl;
        int CHid = FindClusterHead(recieverNodeId);
        if(CHid != -1){
          newHeader = CastHeader(destinationHeader);
          newHeader.SetIsEdgePacket(0);
          newHeader.SetClusterHeadId(CHid);
          newHeader.SetSenderNodeId(recieverNodeId);
          packet->AddHeader(newHeader);
          SendPacket(packet,CHid);
        }
        else{
          //sendpacketcount--;
          newHeader = CastHeader(destinationHeader);
          packet->AddHeader(newHeader);
          SendPacket(packet,newHeader.GetDestinationAddress());
        }
      }
    }
  }
}

static void GenerateHelloBroadcast(Ptr<Socket> socket, uint32_t packetSize, uint32_t packetCount, Time packetInterval)
{
  if(packetCount > 0)
  {
    std::cout<<"Start of Hello broadcast traffic "<<packetCount<<" PACKETS REMAINING"<<std::endl;
    std::cout<<"Sending packet! I am node "<< socket->GetNode ()->GetId () <<std::endl;
    Packet::EnablePrinting ();
    MyHeader sourceHeader;
    CurrentTime = Simulator::Now();
    sourceHeader.SetTimeStamp (CurrentTime);
    sourceHeader.SetResidualEnergy(GetNodeRemainingEnergy(socket->GetNode()->GetId()));
    
    RR[socket->GetNode()->GetId()].TxPackets += 1;
    UpdateReputation();
    sourceHeader.SetMessageType(1);
    sourceHeader.SetReputation(RR[socket->GetNode()->GetId()].Reputation);
    sourceHeader.SetSpeed(GetVelocityCourseChange(socket->GetNode()->GetId()));
    sourceHeader.SetNodeId(socket->GetNode()->GetId());
    Ptr<Packet> p = Create<Packet> (packetSize);
    p->AddHeader (sourceHeader);
    socket->Send (p);
    HeaderType = 0;
    Simulator::Schedule (packetInterval, &GenerateHelloBroadcast, socket, packetSize,
                            packetCount - 1, packetInterval);
                                      totalHelloPackets += 1;
  }
  else
  {
    socket->Close ();
  }
}

static void GenerateHelloCluster(Ptr<Socket> socket, uint32_t packetSize, uint32_t packetCount, Time packetInterval)
{
  if(packetCount > 0)
  {
    std::cout<<"Inside generate cluster time "<<Simulator::Now().GetSeconds()<<std::endl;
    Packet::EnablePrinting ();
    int NodeId = socket->GetNode()->GetId();
    MessageHeader sourceHeader;
    sourceHeader.SetClusterHeadId(NodeId);
    sourceHeader.SetMessageType(1);
    sourceHeader.SetDestinationAddress(0);
    sourceHeader.SetSourceAddress(0);
    sourceHeader.SetIsEdgePacket(0);
    Ptr<Packet> p = Create<Packet> (packetSize);
    p->AddHeader (sourceHeader);
    socket->Send (p);
    HeaderType = 1;
    Simulator::Schedule (packetInterval, &GenerateHelloCluster, socket, packetSize,
                            packetCount - 1, packetInterval);
                                      chtx += 1;
  }
  else
  {
    socket->Close ();
  }
}

static void 
SendTaraficPackets()
{
  //std::cout<<"Inside send traffic "<<Simulator::Now().GetSeconds()<<std::endl;
  int sourceNode , destinationNode, packetid;
 
  sourceNode = rand() % 79 + 0; 
  destinationNode = rand() % 79 + 0;
  packetid = rand() % 1000 + 0;
  while (sourceNode == destinationNode)
  {
    destinationNode = rand() % 79 + 0;
  }
  sendpacketcount ++;
  std::cout<<"Source node "<<sourceNode<<" Destination Node "<<destinationNode<<std::endl;
  int CHeadId = FindClusterHead(sourceNode);
  HeaderType = 2;
  Ptr<Packet> packet = Create<Packet> (packetSize);
  //Case 1:
  //Source Node Cluster Member
  //Destination Node Cluster Head
  if(destinationNode == CHeadId)
  {
    //std::cout<<"Here case 1"<<std::endl;
    MessageHeader TrafficHeader;
    TrafficHeader.SetSenderNodeId(sourceNode);
    TrafficHeader.SetSourceAddress(sourceNode);
    TrafficHeader.SetDestinationAddress(destinationNode);
    TrafficHeader.SetMessageType(2);
    TrafficHeader.SetClusterHeadId(CHeadId);
    TrafficHeader.SetIsEdgePacket(0);
    TrafficHeader.SetPacketId(packetid);
    TrafficHeader.SetTimeStamp(Simulator::Now());
    packet->AddHeader(TrafficHeader);
    SendPacket(packet,CHeadId);
  }

  //Case 2 and 3:
  //Source Node Cluster Head
  //Destination Node In Next Hop Function

  else if(sourceNode == CHeadId){
    NextHop nexthop;
    std::cout<<"Here case 2, 3"<<std::endl;
    //Check in Route
    nexthop = FindNextHop(sourceNode,destinationNode);
    if(nexthop.NextHopType != -1)
    {
      MessageHeader TrafficHeader;
      TrafficHeader.SetSourceAddress(sourceNode);
      TrafficHeader.SetMessageType(2);
      TrafficHeader.SetPacketId(packetid);
      TrafficHeader.SetTimeStamp(Simulator::Now());
      //Case 0 CH is Source and CM is destination
      //Case 1 CH is source and DCH is destination
      if(nexthop.NextHopType == 0 || nexthop.NextHopType == 1){
        TrafficHeader.SetClusterHeadId(CHeadId);
        TrafficHeader.SetDestinationAddress(destinationNode);
        TrafficHeader.SetSenderNodeId(sourceNode);
        TrafficHeader.SetIsEdgePacket(0);
        packet->AddHeader(TrafficHeader);
        // Directly Connected Nodes
        //itself head node send to destination
        SendPacket(packet,nexthop.NextHopId);
      }
      //Case 2 CH is source and twohop is destination
      else if(nexthop.NextHopType == 2){
        TrafficHeader.SetDestinationAddress(destinationNode);
        TrafficHeader.SetClusterHeadId(-1);
        TrafficHeader.SetSenderNodeId(sourceNode);
        TrafficHeader.SetIsEdgePacket(0);
        packet->AddHeader(TrafficHeader);
      
        // Two Hop Node
        //Send packet to next hop
        SendPacket(packet,nexthop.NextHopId);
      }
      //Case 3 CH is source and other CM is destination
      else if(nexthop.NextHopType == 3)
      {
        NextHop dnextHop = FindNextHop(sourceNode,nexthop.NextHopId);
        //if CH is DCH
        if(dnextHop.NextHopType == 1)
        {
          TrafficHeader.SetDestinationAddress(destinationNode);
          TrafficHeader.SetClusterHeadId(dnextHop.NextHopId);
          TrafficHeader.SetSenderNodeId(sourceNode);
          TrafficHeader.SetIsEdgePacket(0);
          packet->AddHeader(TrafficHeader);
          // send  it in other cluster head
          SendPacket(packet,dnextHop.NextHopId);
        }
        // else find edge node
        else
        {
          EdgeNextHop nexthopedge = FindEdgeNode(sourceNode,nexthop.NextHopId);
          if(nexthopedge.EdgeNodeId != -1)
          {
            TrafficHeader.SetDestinationAddress(destinationNode);
            TrafficHeader.SetClusterHeadId(nexthopedge.ClusterHeadId);
            TrafficHeader.SetSenderNodeId(sourceNode);
            TrafficHeader.SetIsEdgePacket(1);
            packet->AddHeader(TrafficHeader);
            // Edge Node
            //Send packet to next hop
            std::cout<<"CH "<<nexthopedge.ClusterHeadId<<" EN "<<nexthopedge.EdgeNodeId<<std::endl;
            SendPacket(packet,nexthopedge.EdgeNodeId);
          }
          else
          {
            TrafficHeader.SetDestinationAddress(destinationNode);
            TrafficHeader.SetClusterHeadId(-4);
            TrafficHeader.SetSenderNodeId(sourceNode);
            TrafficHeader.SetIsEdgePacket(0);
            packet->AddHeader(TrafficHeader);
            // send  it in other cluster head
            SendPacket(packet,destinationNode);
          }
        }
      }
    }
    else{
      //sendpacketcount--;
      MessageHeader TrafficHeader;
      TrafficHeader.SetSourceAddress(sourceNode);
      TrafficHeader.SetMessageType(2);
      TrafficHeader.SetPacketId(packetid);
      TrafficHeader.SetDestinationAddress(destinationNode);
      TrafficHeader.SetClusterHeadId(-4);
      TrafficHeader.SetSenderNodeId(sourceNode);
      TrafficHeader.SetIsEdgePacket(0);
      TrafficHeader.SetTimeStamp(Simulator::Now());
      packet->AddHeader(TrafficHeader);
      // send  it in other cluster head
      SendPacket(packet,destinationNode);
    }
  }
  else if(CHeadId == -1){
    //Check if source is two hop neighbour
    int interNode = FindIntermediateHop(sourceNode);
    if(interNode != -1){
      MessageHeader TrafficHeader;
      TrafficHeader.SetSourceAddress(sourceNode);
      TrafficHeader.SetSenderNodeId(sourceNode);
      TrafficHeader.SetDestinationAddress(destinationNode);
      TrafficHeader.SetMessageType(2);
      TrafficHeader.SetClusterHeadId(-2);
      TrafficHeader.SetIsEdgePacket(0);
      TrafficHeader.SetPacketId(packetid);
      TrafficHeader.SetTimeStamp(Simulator::Now());
      packet->AddHeader(TrafficHeader);
      SendPacket(packet,interNode);
    }
    else{
      //sendpacketcount--;
      MessageHeader TrafficHeader;
      TrafficHeader.SetSourceAddress(sourceNode);
      TrafficHeader.SetSenderNodeId(sourceNode);
      TrafficHeader.SetDestinationAddress(destinationNode);
      TrafficHeader.SetMessageType(2);
      TrafficHeader.SetClusterHeadId(-4);
      TrafficHeader.SetIsEdgePacket(0);
      TrafficHeader.SetPacketId(packetid);
      TrafficHeader.SetTimeStamp(Simulator::Now());
      packet->AddHeader(TrafficHeader);
      SendPacket(packet,destinationNode);
    }
  }
  //Case 4, 5, 6
  //Directly send to cluster head
  else if(CHeadId != -1 && sourceNode != CHeadId && destinationNode != CHeadId)
  {
    //std::cout<<"Here case 4, 5, 6"<<std::endl;
    MessageHeader TrafficHeader;
    TrafficHeader.SetSourceAddress(sourceNode);
    TrafficHeader.SetSenderNodeId(sourceNode);
    TrafficHeader.SetDestinationAddress(destinationNode);
    TrafficHeader.SetMessageType(2);
    TrafficHeader.SetClusterHeadId(CHeadId);
    TrafficHeader.SetIsEdgePacket(0);
    TrafficHeader.SetPacketId(packetid);
    TrafficHeader.SetTimeStamp(Simulator::Now());
    packet->AddHeader(TrafficHeader);
    SendPacket(packet,CHeadId);
  }
  Simulator::Schedule(MilliSeconds(35), &SendTaraficPackets);
}

void SendPacket(Ptr<Packet> packet, int nextHop)
{
  double delay = 1.0+ ((double) rand() / (RAND_MAX));
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Node> node = NodeList::GetNode(nextHop);
  Ptr<Socket> socket = Socket::CreateSocket (node, tid);
  Simulator::Schedule (MilliSeconds (delay), &SendTo, socket, packet, GetNodeAddress(nextHop));
}

void 
SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  InetSocketAddress remote = InetSocketAddress (destination, port);
  socket->Connect(remote);  
  socket->Send (packet);
  if(socket != 0){
    socket->Close();
  }
}

EdgeNextHop 
FindEdgeNode (int Source,int Destination)
{
  int isdirectedge = false;
  int edgenode = -1;
  int clusterhead = -1;
  for(std::vector<int>::size_type i = 0; i != EdgeNodes.size(); i++){
    if(EdgeNodes[i].FirstClusterHeadId == Source && EdgeNodes[i].SecondClusterHeadId == Destination){
      isdirectedge = true;
      edgenode = EdgeNodes[i].EdgeNodeId;
      clusterhead = Destination;
      std::cout<<"inside edge case 1"<<std::endl;
      break;
    }
    else if(EdgeNodes[i].SecondClusterHeadId == Source && EdgeNodes[i].FirstClusterHeadId == Destination){
      isdirectedge = true;
      edgenode = EdgeNodes[i].EdgeNodeId;
      clusterhead = Destination;
      std::cout<<"inside edge case 2"<<std::endl;
      break;
    }
    else if(EdgeNodes[i].SecondClusterHeadId == Destination){
      edgenode = EdgeNodes[i].FirstClusterHeadId;
      isdirectedge = false;
      std::cout<<"inside edge case 3"<<std::endl;
    }
    else if(EdgeNodes[i].FirstClusterHeadId == Destination){
      edgenode = EdgeNodes[i].SecondClusterHeadId;
      isdirectedge = false;
      std::cout<<"inside edge case 4"<<std::endl;
    }
  }
  if(isdirectedge == false){
    if(edgenode != -1){
      for(std::vector<int>::size_type i = 0; i != EdgeNodes.size(); i++){
        if(EdgeNodes[i].FirstClusterHeadId == edgenode && EdgeNodes[i].SecondClusterHeadId == Source){
          edgenode = EdgeNodes[i].EdgeNodeId;
          clusterhead = EdgeNodes[i].FirstClusterHeadId;
          std::cout<<"inside edge case 5"<<std::endl;
          break;
        }
        else if(EdgeNodes[i].SecondClusterHeadId == edgenode && EdgeNodes[i].FirstClusterHeadId == Source){
          edgenode = EdgeNodes[i].EdgeNodeId;
          clusterhead = EdgeNodes[i].SecondClusterHeadId;
          std::cout<<"inside edge case 6"<<std::endl;
          break;
        }
      }
    }
  }
  EdgeNextHop eh;
  eh.ClusterHeadId = clusterhead;
  eh.EdgeNodeId = edgenode;
  return eh;
}

int
FindClusterHead(int SourceNode)
{
  int ClusterHead;
  bool found = false;
  for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
  {
    if(CH[i].ClusterMembersId == SourceNode && CH[i].isJoinedCluster == 1){
      ClusterHead = CH[i].ClusterHeadId;
      found = true;
      break;
    }
    if(CH[i].ClusterHeadId == SourceNode){
      found = true;
      ClusterHead = CH[i].ClusterHeadId;
      break;
    }
  }
  if (found == false){
    ClusterHead = -1;
  }  
  return ClusterHead;
}

NextHop
FindNextHop(int SourceNode, int DestinationNode)
{
  NextHop nexthop;
  //If not found a route return -1 for broadcast
  nexthop.NextHopId = -1;
  nexthop.NextHopType = -1;
  for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
  {
    //Case 0 CH is Source and CM is destination
    if(CH[i].ClusterHeadId == SourceNode && CH[i].ClusterMembersId == DestinationNode && CH[i].isJoinedCluster){
      nexthop.NextHopId = CH[i].ClusterMembersId;
      nexthop.NextHopType = 0;
      break;
    }
    //Case 1 CH is source and DCH is destination
    if(CH[i].ClusterHeadId == SourceNode && CH[i].DClusterHeadId == DestinationNode && CH[i].isDClusterHead == 1){
      nexthop.NextHopId = CH[i].DClusterHeadId;
      nexthop.NextHopType = 1;
      break;
    }
    //Case 2 CH is source and twohop is destination
    if(CH[i].ClusterHeadId == SourceNode && CH[i].isSecondHop && CH[i].TwoHopMember == DestinationNode){
      nexthop.NextHopId = CH[i].IntermediateMember;
      nexthop.NextHopType = 2;
      break;
    }
    //Case 3 CH is source and other CM is destination
    if(CH[i].ClusterMembersId == DestinationNode && CH[i].ClusterHeadId != SourceNode){
      nexthop.NextHopId = CH[i].ClusterHeadId;
      std::cout<<"Destination "<<DestinationNode<<" nexthop "<<CH[i].ClusterHeadId<<std::endl;
      nexthop.NextHopType = 3;
      break;
    }
    //Case 4 Twohop memeber in other cluster is destination
    if(CH[i].TwoHopMember == DestinationNode){
      nexthop.NextHopId = CH[i].ClusterHeadId;
      nexthop.NextHopType = 4;
    }
  }
  return nexthop;
}

int
FindIntermediateHop(int SourceNode)
{
  int IntermediateMember;
  bool found = false;
  for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
  {
    if(CH[i].TwoHopMember == SourceNode){
      IntermediateMember = CH[i].IntermediateMember;
      found = true;
      break;
    }
  }
  if (found == false){
    IntermediateMember = -1;
  }  
  return IntermediateMember;
}

static void 
SendClusterJoinRequest()
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  static const InetSocketAddress Broadcast = InetSocketAddress(Ipv4Address("255.255.255.255"), port);
  for (uint32_t j=0; j<16; j++)
  {
    Ptr<Node> node = NodeList::GetNode(clusterHeadId[j]);
    Ptr<Socket> source = Socket::CreateSocket (node, tid);
    source->SetAllowBroadcast (true);
    source->Connect (Broadcast);
    double t =  0.001+ ((double) rand() / (RAND_MAX));
    Time pki = MilliSeconds(30);
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (), Seconds(t), &GenerateHelloCluster, source, packetSize, packetCount, pki);
  } 
}


//Tracing Functions Section

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
   UP[std::stoi(context)].CourseChangeCount += 1;
   if(UBCourseChangeCount < UP[std::stoi(context)].CourseChangeCount){
     UBCourseChangeCount = UP[std::stoi(context)].CourseChangeCount;
     bounds_matrix[6][1] = UBCourseChangeCount;
   }
}

void 
CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;
  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);
  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_txp << ""
      << std::endl;
  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &CheckThroughput);
}

std::vector<Neighbours>
CheckorAddNeighbours(int recieverId , int senderId, std::vector<Neighbours> nodeIdList)
{
  Neighbours tempneighbour;
  tempneighbour.NodeId = senderId;
  tempneighbour.NeighboursId = recieverId;
  nodeIdList.push_back(tempneighbour);
  return nodeIdList;
}

void
CheckorAddTwoHopClusterMembers()
{
  std::cout<<"Two hop function "<<CH.size()<<std::endl;
  int htempNodes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79};

  for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
  {
    for(int j = 0; j < 80; j++)
    {
      if(CH[i].ClusterHeadId == htempNodes[j]){
        htempNodes[j] = -1;
      }
      else if(CH[i].ClusterMembersId == htempNodes[j]){
          htempNodes[j] = -1;
      }
    }
  }

  std::vector<Neighbours> tempunclusternodes;
  for(int j = 0; j < 80; j++){
    if(htempNodes[j] != -1){
      for(std::vector<int>::size_type k = 0; k != NodeIds.size(); k++)
      {
        if(NodeIds[k].NodeId == htempNodes[j]){
          Neighbours tempneighbours;
          tempneighbours.NodeId = NodeIds[k].NodeId;
          tempneighbours.NeighboursId = NodeIds[k].NeighboursId;
          tempunclusternodes.push_back(tempneighbours);
        }
      }
    }
  }

  for(std::vector<int>::size_type u = 0; u != tempunclusternodes.size(); u++)
  {
    for(std::vector<int>::size_type c = 0; c != CH.size(); c++)
    {
      if(tempunclusternodes[u].NeighboursId == CH[c].ClusterMembersId){
        CH[c].isSecondHop = 1;
        CH[c].TwoHopMember = tempunclusternodes[u].NodeId;
        CH[c].IntermediateMember = tempunclusternodes[u].NeighboursId;
        break;
      }
    }
  }
  //for(std::vector<int>::size_type i = 0; i != CH.size(); i++)
  //{
    //std::cout<<"CH "<<CH[i].ClusterHeadId<<" CM "<<CH[i].ClusterMembersId <<" DCH "<<CH[i].DClusterHeadId<<" THopM "<<CH[i].TwoHopMember<<" IM "<<CH[i].IntermediateMember<<std::endl;
  //}
}

// Get Functions Sections

double
GetVelocityCourseChange(int nodeId)
{
  Ptr<Node> node = NodeList::GetNode (nodeId);
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->TraceConnect ("CourseChange",GetStringNodeId(node), MakeCallback (&CourseChange));
  Vector velocity = mobility->GetVelocity();
  return std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
}

double
GetNodeRemainingEnergy(int nodeId)
{
   Ptr<Node> node = NodeList::GetNode (nodeId);
   Ptr<EnergySourceContainer> sources = node->GetObject<EnergySourceContainer> ();
   Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources->Get(0));
   NS_ASSERT (basicSourcePtr != NULL);
   return basicSourcePtr->GetRemainingEnergy();
}

// Set Values Functions Section

void
InitializeGlobalValues()
{
    port = 9;
    bytesTotal = 0;
    packetsReceived = 0;
    m_CSVfileName ="fanet-routing.output.csv";
    gNodes = 80;
    m_nSinks = 10;
    m_txp = 7.5;
    UAVEnergy  = 70;
    TotalTime = 100.0;
    rate = "2048bps";
    PhyMode = "ErpOfdmRate54Mbps";
    tr_name = "fanet-routing";
    packetCount = 1;
    packetInterval = MilliSeconds (25);
    packetSize = 512;
    CurrentTime = Simulator::Now();
    totalHelloPackets = 0;
}

// Install Models on Nodes Function Section

void
InstallEnergyModel(NodeContainer & uavNodes, NetDeviceContainer & UAVadhocDevices)
{
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (UAVEnergy));
    EnergySourceContainer sources = basicSourceHelper.Install (uavNodes);
    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.067));
    radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.046));
    radioEnergyHelper.Set ("IdleCurrentA", DoubleValue(0.01));
    radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue(0.01));
    radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue(0.01));
    radioEnergyHelper.Set ("SleepCurrentA", DoubleValue(0.001));
    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (UAVadhocDevices, sources);
}

void
InstallMobilityModel(NodeContainer & uavNodes)
{
 MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  pos.Set ("Z", StringValue ("100.0"));
  
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=20.0|Max=" << 50 << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << 1 << "]";
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (uavNodes);
  streamIndex += mobilityAdhoc.AssignStreams (uavNodes, streamIndex);
  NS_UNUSED (streamIndex); // From this point, streamIndex is unused
  
  /*
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
    "Bounds", BoxValue (Box (-2000, 2000, -100, 100, 100, 100)),
    "TimeStep", TimeValue (Seconds (2)),
    "Alpha", DoubleValue (0.50),
    "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=100|Max=150]"),
    "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0.183185307]"),
    "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
    "NormalVelocity", 
    StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
    "NormalDirection",
    StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.1|Bound=0.3]"),
    "NormalPitch", 
    StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
    mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
    "X", StringValue ("ns3::UniformRandomVariable[Min=-100|Max=100]"),
    "Y", StringValue ("ns3::UniformRandomVariable[Min=-100|Max=100]"),
    "Z", StringValue ("ns3::UniformRandomVariable[Min=100|Max=100]"));
    mobility.Install (uavNodes);
*/
}

// Cluster Head Selection EABC 

static void 
ClusterHeadSelection ()
{
  for(int a=0; a< 16; a++)
  {
    InitPopulation();
  }
  double nowtime = Simulator::Now().GetSeconds();
  ClusterHeadAnnouncemenTimer = nowtime ;
  std::cout<<"Inside Clusterhead selection time is "<<nowtime<<std::endl;
  SendClusterJoinRequest();
  
}

void InitPopulation(){

	int cycle = 0;

	// initialize food source 

  init();
	g_best_value = fitness_array[0];
  g_best_index = 0;
  get_gbest();
	while (cycle < MAX_NUM_CYCLES) {
		send_employed_bees();
		calculate_probabilities();
		send_onlooker_bees();
		send_scout_bees();
    get_gbest();
		cycle++;
	}

  clusterHeadId[chCount] = g_best_index;
  chCount++;
}

void init() {
	for (int i = 0; i < FOOD_SOURCES_SIZE; i++) {
		init_bee(i);
	}
}

double calculate_function(double solution[PARAMS_SIZE]) 
{
  return solution[0] + solution[1] + solution[2] + solution[3] / solution[4] + solution[5] + solution[6];
}

double calculate_fitness(double value) {
	if (value >= 0) {
	  return 1 / (value + 1);
	} else {
	  return 1 + abs((long)value);
	}

}

double get_random_number() {
	return ((double) rand() / ((double) (RAND_MAX) + (double) (1)));
}

void init_bee(int index) {
	// food source array
	double solution[PARAMS_SIZE];

	// generate food source using upper and lower bounds 
	for (int j = 0; j < PARAMS_SIZE; j++) {
    if(j == 0){foods_matrix[index][j] = UP[index].RemainingEnergy;}
    else if(j == 1){foods_matrix[index][j] = UP[index].ReputationRanking;}
    else if(j == 2){foods_matrix[index][j] = UP[index].TotalOnlineTime;}
    else if(j == 3){foods_matrix[index][j] = UP[index].Transactions;}
    else if(j == 4){foods_matrix[index][j] = UP[index].Latency;}
    else if(j == 5){foods_matrix[index][j] = UP[index].Speed;}
    else if(j == 6){foods_matrix[index][j] = UP[index].CourseChangeCount;}
		if(chCount>0){
for(int ch = 0; ch<chCount; ch++){
     int id = clusterHeadId[ch];
     foods_matrix[id][0] = 0;
     foods_matrix[id][1] = 0;
     foods_matrix[id][2] = 0;
     foods_matrix[id][3] = 0;
     foods_matrix[id][4] = UBLatency;
     foods_matrix[id][5] = UBSpeed;
     foods_matrix[id][6] = UBCourseChangeCount;
}
    }
		solution[j] = foods_matrix[index][j];
	}

	//calculate objective function value
	function_array[index] = calculate_function(solution);
	
	//calculate fitness function value
	fitness_array[index] = calculate_fitness(function_array[index]);

	//set trail count to zero
	trail_count_array[index] = 0;
}

void send_employed_bees() {
	double new_solution[PARAMS_SIZE];
	int neighbour = 0;

	for (int i = 0; i < FOOD_SOURCES_SIZE; i++) {
		double r = get_random_number();
		int param_to_modify = (int) (r * PARAMS_SIZE);

		do {
			r = get_random_number();
			neighbour = (int) (r * FOOD_SOURCES_SIZE);
		} while (neighbour == i);

		for (int j = 0; j < PARAMS_SIZE; j++) {
			new_solution[j] = foods_matrix[i][j];
		}

		r = get_random_number();
		new_solution[param_to_modify] = foods_matrix[i][param_to_modify]
				+ (foods_matrix[i][param_to_modify]
						- foods_matrix[neighbour][param_to_modify]) * ((r - 0.5)
						* 2) + (foods_matrix[g_best_index][param_to_modify] 
            - foods_matrix[i][param_to_modify]);

		if (new_solution[param_to_modify]
				< bounds_matrix[param_to_modify][LOWER_BOUND]) {
			new_solution[param_to_modify]
					= bounds_matrix[param_to_modify][LOWER_BOUND];
		}
		if (new_solution[param_to_modify]
				> bounds_matrix[param_to_modify][UPPER_BOUND]) {
			new_solution[param_to_modify]
					= bounds_matrix[param_to_modify][UPPER_BOUND];
		}

		double new_solution_function = calculate_function(new_solution);
		double new_solution_fitness = calculate_fitness(new_solution_function);

		/* verificar se a nova solucao e melhor que a atual */
		//-------------------------------------------------- MAX < / MIN >
		if (new_solution_fitness > fitness_array[i]) {
			trail_count_array[i] = 0;
			for (int j = 0; j < PARAMS_SIZE; j++) {
				foods_matrix[i][j] = new_solution[j];
			}
			function_array[i] = new_solution_function;
			fitness_array[i] = new_solution_fitness;
		} else {
			trail_count_array[i] += 1;
		}
	}
}

void calculate_probabilities() {
	// pegar o maior fitness
	double maxfit = fitness_array[0];
	for (int i = 1; i < FOOD_SOURCES_SIZE; i++) {
		if (fitness_array[i] > maxfit) {
			maxfit = fitness_array[i];
		}
	}

	for (int i = 0; i < FOOD_SOURCES_SIZE; i++) {
		probabilities_array[i] = (0.9 * (fitness_array[i] / maxfit)) + 0.1;
	}
}

void send_onlooker_bees() {
	int i = 0;
	int t = 0;
	double new_solution[PARAMS_SIZE];
	while (t < FOOD_SOURCES_SIZE) {
		double r = get_random_number();
		if (r < probabilities_array[i]) {
			t++;

			r = get_random_number();
			int param_to_modify = (int) (r * PARAMS_SIZE);

			for (int j = 0; j < PARAMS_SIZE; j++) {
				new_solution[j] = foods_matrix[i][j];
			}

			r = get_random_number();
			new_solution[param_to_modify] = foods_matrix[g_best_index][param_to_modify]
					+ (foods_matrix[g_best_index][param_to_modify]
							- foods_matrix[i][param_to_modify]) * (r
							- 0.5) * 2;

			if (new_solution[param_to_modify]
					< bounds_matrix[param_to_modify][LOWER_BOUND]) {
				new_solution[param_to_modify]
						= bounds_matrix[param_to_modify][LOWER_BOUND];
			}
			if (new_solution[param_to_modify]
					> bounds_matrix[param_to_modify][UPPER_BOUND]) {
				new_solution[param_to_modify]
						= bounds_matrix[param_to_modify][UPPER_BOUND];
			}

			double new_solution_function = calculate_function(new_solution);
			double new_solution_fitness = calculate_fitness(
					new_solution_function);

			if (new_solution_fitness > fitness_array[i]) {
				trail_count_array[i] = 0;
				for (int j = 0; j < PARAMS_SIZE; j++) {
					foods_matrix[i][j] = new_solution[j];
				}
				function_array[i] = new_solution_function;
				fitness_array[i] = new_solution_fitness;
			} else {
				trail_count_array[i] += 1;
			}
		}
		i++;
		if (i == FOOD_SOURCES_SIZE) {
			i = 0;
		}
	}
}

void send_scout_bees() {
	int max_trial_index = 0;
	for (int i = 1; i < FOOD_SOURCES_SIZE; i++) {
		if (trail_count_array[i] > trail_count_array[max_trial_index]) {
			max_trial_index = i;
		}
	}
	if (trail_count_array[max_trial_index] >= LIMIT) {
		init_bee(max_trial_index);
	}
}

void get_gbest() {
	for (int i = 0; i < FOOD_SOURCES_SIZE; i++) {
		if (fitness_array[i] < g_best_value) {
			g_best_value = fitness_array[i];
			g_best_index = i;
		}
	}
// std::cout<<"best value "<<g_best_value << "best index "<<g_best_index<<std::endl;
}

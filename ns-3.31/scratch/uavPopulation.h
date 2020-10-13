#ifndef UAVPOPULATION_H
#define UAVPOPULATION_H
#include "ns3/core-module.h"
#include <vector> 

using namespace ns3;

#define PI 3.14159265
#define POPULATION_SIZE 160
#define MAX_NUM_CYCLES 10
#define PARAMS_SIZE 7
#define FOOD_SOURCES_SIZE POPULATION_SIZE/2
#define LIMIT (POPULATION_SIZE*PARAMS_SIZE)/2
#define UPPER_BOUND 1
#define LOWER_BOUND 0

struct UAVPopulation {
  int NodeId;
  Ipv4Address IpAddress;
  double RemainingEnergy;
  double ReputationRanking;
  double TotalOnlineTime;
  int Transactions;
  double Latency;
  double Speed;
  int CourseChangeCount;
  double SimulationTime;

} UP[80];

struct ReputationRanking{
  int NodeId;
  Ipv4Address IpAddress;
  int RxPackets;
  int TxPackets;
  int DrpPackets;
  double Reputation;
  double SimulationTime;
} RR[80];

struct Neighbours
{
    int NodeId;
    int NeighboursId;
};

std::vector<Neighbours> NodeIds;

struct NextHop
{
    int NextHopId;
    int NextHopType;
};

std::vector<int> PacketTracker;

struct EdgeNode
{
    int FirstClusterHeadId;
    int SecondClusterHeadId;
    int EdgeNodeId;
};

std::vector<EdgeNode> EdgeNodes;

struct EdgeNextHop{
  int ClusterHeadId;
  int EdgeNodeId;
};

struct KeyPairHex {
  std::string publicKey;
  std::string privateKey;
};

struct KeyPair{
  std::string PublicKey;
  std::string PrivateKey;
  int NodeId;
};

std::vector<KeyPair> KeyPairs;

struct Transaaction{
  int TransactionId;
  int PreviousTransactionId;
  std::string PublicKey;
  Time TimeStamp;
  std::string Signature;
  double Reputation;
  std::string NextPublicKey;
  std::string Hash;
  UAVPopulation TransactionMetadata;
};

std::vector<Transaaction> Transaactions;

struct Block{
  std::string Hash;
  std::string PreviousHash;
  std::string WitnessSignature;
  Time TimeStamp;
  std::vector<Transaaction> Transactions;
};

std::vector<Block> Blocks;

// Upper Bound Values 
double UBRemainingEnergy = 0;
double UBReputationRanking = 0;
double UBTotalOnlineTime = 0;
double UBTransactions = 0;
double UBLatency = 0;
double UBSpeed = 0;
double UBCourseChangeCount = 0;

//Global Veriables 

int gNodes;
uint32_t port;
uint32_t bytesTotal;
uint32_t packetsReceived;
int m_nSinks;
double m_txp;
std::string m_CSVfileName;
double UAVEnergy;
double TotalTime;
std::string rate;
std::string PhyMode;
std::string tr_name;
uint32_t packetSize; 
uint32_t packetCount; 
Time packetInterval;
const double maxRange = 90.0;
int totalHelloPackets;
int clusterHeadId[16];
int chCount  = 0;
int eabcCount = 0;
int rpacketlast =0;
int chtx = 0;
int TransactionCount = 0;
int BlockCount = 0;
double HelloTimer;
double ClusterHeadSelectionTimer;
double ClusterHeadAnnouncemenTimer;
double TrafficTimer;
double TwoHopTimer;
double r;
double etoedelay = 0;
int recievepacketcount = 0;
int sendpacketcount = 0;
EventId ClusterHeadSelectionEventId;
EventId TrafficEventId;

double bounds_matrix[PARAMS_SIZE][2] = { { 0, UBRemainingEnergy }, { 0, UBReputationRanking }, { 0, UBTotalOnlineTime }, { 0, 10 }, { 0, UBLatency }, { 0, UBSpeed }, { 0, UBCourseChangeCount } };
double foods_matrix[FOOD_SOURCES_SIZE][PARAMS_SIZE];
double function_array[FOOD_SOURCES_SIZE];
double fitness_array[FOOD_SOURCES_SIZE];
double trail_count_array[FOOD_SOURCES_SIZE];
double probabilities_array[FOOD_SOURCES_SIZE];
double optimum_solution;
double optimun_params_array[PARAMS_SIZE];
int g_best_index;
double g_best_value;

#endif
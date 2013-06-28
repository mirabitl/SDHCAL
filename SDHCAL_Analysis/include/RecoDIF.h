#ifndef _RECODIF_H
#define _RECODIF_H
#include <iostream>
#include <vector>
#include <map>
/**
   Definition of a RecoHitAddress as a pair of unsigned char
 */
typedef std::pair<unsigned char,unsigned char> RecoHitAddress;


/**
   \class RecoCluster
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief Vector of RecoHitAddress. The position is the mean of X and Y. No usage of threshold
 */
class RecoCluster
{
  public:
  RecoCluster(RecoHitAddress h);
  ~RecoCluster();
  unsigned int dist(RecoHitAddress h1,RecoHitAddress h2);
  bool Append(RecoHitAddress h);
  std::vector<RecoHitAddress>* getHits();
  float Pos(int p);
  void Print();
  float X();
  float Y();
private:
  std::vector<RecoHitAddress> hits_;
};

/**
   \class RecoDIF
   \author L.Mirabito
   \date May 2010
   \version 1.0
   \brief Vector of RecoCluster. Might need a rewritting  per chamber
 */


class RecoDIF
{
public:
  RecoDIF(unsigned int id);
  ~RecoDIF(); 
  void AddHit(RecoHitAddress h);
  std::vector<RecoCluster>* getClusters();
  unsigned int getDifId();
  void Print();
private:
  unsigned int difid_;
  std::vector<RecoCluster> clusters_; 
};

#endif

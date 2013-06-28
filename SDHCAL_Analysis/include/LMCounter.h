#ifndef _LMCounter_H_
#define _LMCounter_H_
#include <map>

#include <string>
#include <iostream>

class LMCounter
{
 public:
  LMCounter()
    {
      cntr_.clear();
    }

  ~LMCounter()
    {
      cntr_.clear();

    }
  void addCounter(std::string name,int size=1)
  {
    std::vector<unsigned int> v(size);
    for (int i=0;i<size;i++) v[i]=0;
    std::pair<std::string, std::vector<unsigned int> > pr(name,v);
    cntr_.insert(pr);
  }

  int increment(std::string name, int index=0,int weight=1)
    {
      std::map<std::string,std::vector<unsigned int> >::iterator iter= cntr_.find(name);
      if (iter!=cntr_.end())
	{
	  iter->second[index] +=weight;
	  return iter->second[index];
	}
      else
	return -1;
    }
  std::vector<unsigned int>* getCounters(std::string name)
    {
      std::map<std::string,std::vector<unsigned int> >::iterator iter= cntr_.find(name);
      if (iter==cntr_.end()) return 0;
      return &iter->second;
    }

  int getCounter(std::string name,int index=0)
  {
    std::map<std::string,std::vector<unsigned int> >::iterator iter= cntr_.find(name);
    if (iter==cntr_.end()) return -1;
    return iter->second[index];
  }
  std::vector<std::string> getNames()
    {
      std::vector<std::string> v;
       for (std::map<std::string,std::vector<unsigned int> >::iterator iter=cntr_.begin();iter!=cntr_.end();iter++)
      {
	v.push_back(iter->first);
      }
       return v;
    }
  void Print()
  {
    std::cout<<std::endl;
    for (std::map<std::string,std::vector<unsigned int> >::iterator iter=cntr_.begin();iter!=cntr_.end();iter++)
      {
	std::cout<<"Name : "<<iter->first<<std::endl;
	std::cout<<"Content : ";
	for (unsigned int j=0;j<iter->second.size();j++) std::cout<<iter->second[j]<<" ";
	std::cout<<std::endl;
      }
   
  }
  
 private:
  std::map<std::string,std::vector<unsigned int> > cntr_;
};
#endif

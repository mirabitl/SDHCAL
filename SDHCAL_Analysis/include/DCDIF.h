#ifndef _DCDIF_H
#define _DCDIF_H
#include <stdint.h>
/** 
\class DCDIF
  \author  L.Mirabito 
  \date March 2010
  \version 1.0
  
 \brief Handler of DIF info

*/
class DCDIF
{
public:
  //! Cosntructor
  /**
     @param DIdi DIF id
     @param DTCi DIF trigger counter
     @param ATCi Absolute trigger counter
     @param GTCi Global trigger counter
     @param TBC Trigger Bunch Crossing
     @param T_LargeBC  Large Bunch Crossing

   */
 
  DCDIF(unsigned short DIdi,unsigned long DTCi,unsigned long ATCi,unsigned long GTCi, double TBC, double T_LargeBC)
  {_DIFId=DIdi; _DTC=DTCi; _ATC=ATCi; _GTC=GTCi; _TriggerBC=TBC; _LargeBC=T_LargeBC;}
  
  //! Copy Constructor
  DCDIF(const DCDIF &DIFeq)
    {
      _DIFId=DIFeq.getId(); _DTC=DIFeq.getDTC(); _ATC=DIFeq.getATC(); _GTC=DIFeq.getGTC(); _TriggerBC=DIFeq.getTBC(); _LargeBC=DIFeq.getLBC();
    }

  //! Empty Constructor
  DCDIF()
  {_DIFId=0; _DTC=0; _ATC=0; _GTC=0; _TriggerBC=0; _LargeBC=0;}


  //! Set DTC
  /**
     @param DTCi DIF trigger counter
   */
  void setDTC(unsigned long DTCi)			{_DTC=DTCi;}

  //! Set GTC
  /**
     @param GTCi Global trigger counter
   */
  void setGTC(unsigned long GTCi)			{_GTC=GTCi;}

  //! Set TBC
  /**
     @param TBC Global Bunch Crossing
   */
  void setTBC(double TBC)			{_TriggerBC=TBC;}	

  //! Set LBC
  /**
     @param T_LargeBC Large Bunch Crossing
   */
  void	setLBC(double T_LargeBC)		{_LargeBC=T_LargeBC;}
	
  //! Get ID
  unsigned short getId()					const	{return _DIFId;} 

  //! Get DTC
  unsigned long getDTC()					const	{return _DTC;} 

  //! Get ATC
  unsigned long	getATC()					const	{return _ATC;} 

  //! Get GTC
  unsigned long	getGTC()					const	{return _GTC;} 

  //! Get TBC
  double getTBC()				const	{return _TriggerBC;}

  //! Get LBC
  double getLBC()				const	{return _LargeBC;}
  
  //! = Operator
  DCDIF operator=(const DCDIF &DIFeq)
    {
      
      if (&DIFeq==this) return *this;
      _DIFId=DIFeq.getId(); _DTC=DIFeq.getDTC(); _ATC=DIFeq.getATC(); _GTC=DIFeq.getGTC(); _TriggerBC=DIFeq.getTBC(); _LargeBC=DIFeq.getLBC();
      return (*this);
    }	

  inline void setLBA(uint32_t lba ) {_lba=lba;}
  inline void setLBB(uint32_t lbb ) {_lbb=lbb;}
  inline uint32_t getLBA(){return _lba;}
  inline uint32_t getLBB(){return _lbb;}
  //Set all apramters
  void setAll(unsigned short DIdi,unsigned long DTCi,unsigned long ATCi,unsigned long GTCi, double TBC, double T_LargeBC)
  {_DIFId=DIdi; _DTC=DTCi; _ATC=ATCi; _GTC=GTCi; _TriggerBC=TBC; _LargeBC=T_LargeBC;}
 private:
  unsigned short			_DIFId;
  unsigned long				_DTC;
  unsigned long				_ATC;
  unsigned long				_GTC;
  double				_TriggerBC;
  double				_LargeBC;
  uint32_t _lba;
  uint32_t  _lbb;
};
#endif

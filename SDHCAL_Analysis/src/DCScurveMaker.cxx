#include "DCScurveMaker.h"
#include <sstream>
#include "TSystem.h"
#include "TStyle.h"
//#define DEBUG 1
//#define DEBUG_ALL 1
//#define DEBUG_KEYS 1

DCScurveMaker::DCScurveMaker(DHCalEventReader* r,DCHistogramHandler* h){
  hrtype_=2;
  reader_=r;
  handler_=h;
  InjectedDif_.clear();
  InjectedPadVector_.clear();
  InjectedPad_.resize(64);
  PadGain_=0;
  LowThresh_=0; 
  UsedGains_.clear();
}
DCScurveMaker::~DCScurveMaker()
{
  return;
}

void DCScurveMaker::initHistograms(int DifId,int AsicId)
{
#ifdef DEBUG
  cout<<"Booking histos for asic "<<AsicId<<" Dif:"<<DifId<<endl;
#endif
  if ( AsicId < 0 || PadGain_ == 0 || DifId<0) return;
  // no loop on asics as some asics are dead
  gStyle->SetPalette(64,0);
  stringstream control;
  control<<"/DIF_"<<DifId<<"/Asic_"<<AsicId<<"/Gain_"<<PadGain_<<"/Control";
  TH2F *controlHist=(TH2F*)handler_->BookTH2(control.str(),64,0.5,64.5,64,0.5,64.5);
  controlHist->GetXaxis()->SetTitle("Injected Pad");
  controlHist->GetYaxis()->SetTitle("Hit Pad");
  std::map<short,vector<short> >::iterator padmap=ActivePads_.find(AsicId);
  if ( padmap !=ActivePads_.end() ) {
    for ( vector<short>::iterator pad=(padmap->second).begin();pad!=(padmap->second).end();pad++) {
      stringstream scurve, pedestal;
      scurve<<"/DIF_"<<DifId<<"/Asic_"<<AsicId<<"/Gain_"<<PadGain_<<"/Scurve/Pad_"<<*pad+1;
      pedestal<<"/DIF_"<<DifId<<"/Asic_"<<AsicId<<"/Gain_"<<PadGain_<<"/Pedestal/Pad_"<<*pad+1;
      //The histograms do not exist, so we are at the lowest threshold
      TH1F* scurveHist=(TH1F*)handler_->BookTH1(scurve.str(),380-LowThresh_,LowThresh_,380);
      scurveHist->SetLineWidth(2);
      scurveHist->SetLineColor(gStyle->GetColorPalette(*pad));
      scurveHist->GetXaxis()->SetTitle("Threshold");
      scurveHist->GetYaxis()->SetTitle("Nb of times ON");
      TH1F* pedestalHist=(TH1F*)handler_->BookTH1(pedestal.str(),380-LowThresh_,LowThresh_,380);
      pedestalHist->SetLineWidth(4);
      pedestalHist->SetLineColor(gStyle->GetColorPalette(*pad));
      pedestalHist->GetXaxis()->SetTitle("Threshold");
      pedestalHist->GetYaxis()->SetTitle("Nb of times ON");
    }
  }
#ifdef DEBUG
  cout<<"DCScurve exit booking histo"<<endl;
#endif

  return;
}

void DCScurveMaker::processRunHeader()
{
  StringVec intKeys;
#ifdef DEBUG
  cout <<"In processRunHeader"<<endl;
#endif
  ActivePads_.clear();
  IntVec hrVec ;
  reader_->getRunHeader()->getParameters().getIntVals("HardRoc", hrVec ) ;
  hrtype_=hrVec[0];
  int nIntParameters = reader_->getRunHeader()->getParameters().getIntKeys( intKeys ).size() ;
#ifdef DEBUG
  cout <<"found "<<nIntParameters<<" parameters in runheader"<<endl;
#endif
  for(int i=0; i< nIntParameters ; i++ ){
#ifdef DEBUG_KEYS
    cout <<"key "<<i<<" is="<<intKeys[i].c_str() << endl;
#endif
    if (strstr(intKeys[i].c_str(),"Mask") != NULL) {
      IntVec intVec;
      reader_->getRunHeader()->getParameters().getIntVals(  intKeys[i], intVec ) ;
#ifdef DEBUG_ALL
      cout << "mask value=="<< intVec[0] <<endl;
#endif
      int len=intKeys[i].length();
      int pos=intKeys[i].find("C");
      string temp=intKeys[i].substr(pos+2,len-pos-2);
      pos=temp.find("_");
      short a=atoi((temp.substr(0,pos)).c_str());
      if (intVec[0] != 0 ) {
#ifdef DEBUG_ALL
	cout <<"Asic "<<a;
#endif
	if ( ActivePads_.find(a) == ActivePads_.end() ) {
	  vector<short> apads;
	  pair<short,vector<short> > p(a,apads);
	  ActivePads_.insert(p);
#ifdef DEBUG_ALL
	  cout<<" - new pair in map ";
#endif
	}
	pos=temp.find("l");
	len=temp.length();
	temp=temp.substr(pos+2,len-pos-2);
	pos=temp.find("_");
	short p=atoi((temp.substr(0,pos)).c_str());
#ifdef DEBUG_ALL
	cout << " - pushing pad "<<p;
#endif
	vector<short> v=ActivePads_.find(a)->second;
	if ( std::find(v.begin(),v.end(),p) == v.end()) ActivePads_.find(a)->second.push_back(p);
         
#ifdef DEBUG_ALL
	cout << " - total pads is asic: "<<(ActivePads_.find(a)->second).size()<<endl;
#endif
      } else if ( ActivePads_.find(a) != ActivePads_.end() ) {
	pos=temp.find("l");
	len=temp.length();
	temp=temp.substr(pos+2,len-pos-2);
	pos=temp.find("_");
	short p=atoi((temp.substr(0,pos)).c_str());
	vector<short>::iterator vit=std::find((ActivePads_.find(a)->second).begin(),(ActivePads_.find(a)->second).end(),p);
	if ( vit != (ActivePads_.find(a)->second).end()) (ActivePads_.find(a)->second).erase(vit);
      }
    }
    else if (strstr(intKeys[i].c_str(),"cTest") != NULL) {
      IntVec intVec ;
#ifdef DEBUG_ALL
      cout<<" found a cTest param "<<endl;
#endif  
      reader_->getRunHeader()->getParameters().getIntVals(  intKeys[i], intVec ) ;
      int len=intKeys[i].length();
      string temp=intKeys[i].substr(4,len-10);
      int pos=temp.find("_");
      int thisdif=atoi(temp.substr(0,pos).c_str());
      if (std::find(InjectedDif_.begin(),InjectedDif_.end(),thisdif)==InjectedDif_.end())InjectedDif_.push_back(thisdif);
      while (pos>0) {
	temp=temp.substr(pos+1,temp.length()-pos);
	pos=temp.find("_");
      }
#ifdef DEBUG
      cout<<"le pad="<<temp.c_str()<<endl;
#endif
      InjectedPad_[atoi(temp.c_str())]=(bool)intVec[0];
      if ( intVec[0] == 1 )  {
	if (std::find(InjectedPadVector_.begin(),InjectedPadVector_.end(),thisdif)==InjectedPadVector_.end())InjectedPadVector_.push_back(atoi(temp.c_str()));
#ifdef DEBUG
	cout<<"found pad: len="<<len<<" str="<<temp.c_str()<<" pos="<<pos<<" dif="<<temp.substr(0,pos).c_str()<<endl;
#endif
	string gainKey=intKeys[i].substr(0,len-5)+"Gain";
	intVec.clear();
#ifdef DEBUG
	cout<<"looking for gain key="<<gainKey.c_str()<<endl;
#endif
	reader_->getRunHeader()->getParameters().getIntVals(  gainKey, intVec ) ;
	if (intVec.size() > 0 ) PadGain_=intVec[0];
	else cout<<"problem could not find "<<gainKey.c_str()<<endl;
	vector<short>::iterator Git=UsedGains_.begin();
	while ( Git != UsedGains_.end() && *Git != PadGain_){
	  ++Git;
	}
	if ( Git == UsedGains_.end() ) UsedGains_.push_back(PadGain_);
        
	string DacKey=intKeys[i].substr(0,intKeys[i].find("Channel"))+"DAC1";
	intVec.clear();
#ifdef DEBUG
        cout<<"looking for DAC key="<<DacKey.c_str()<<endl;
#endif
	reader_->getRunHeader()->getParameters().getIntVals(  DacKey, intVec ) ;
	if ( intVec.size() == 0 ) {
	  string DacKey2=intKeys[i].substr(0,intKeys[i].find("Channel"))+"B0";
#ifdef DEBUG
	  cout<<"looking for DAC key 2="<<DacKey2.c_str()<<endl;
#endif
	  reader_->getRunHeader()->getParameters().getIntVals(  DacKey2, intVec ) ;
	}
	if (intVec.size()>0 ) LowThresh_=intVec[0];
	else cout <<"problem finding DacKey"<<endl;
	//break;
      }
    }
  }
}

void DCScurveMaker::processEvent()
{  
#ifdef DEBUG
  cout << "DCScurveMaker in processEvent"<<endl;
#endif
  processRunHeader();
  if (InjectedDif_.size()==0)return;
  LCCollection* rhcol = reader_->getEvent()->getCollection("DHCALRawHits");
  if (rhcol == NULL) return;
#ifdef DEBUG
  cout <<" about to fill "<<rhcol->getNumberOfElements()<<" hits"<<endl;
#endif
  for (int i=0;i<rhcol->getNumberOfElements();i++)
    {
      EVENT::RawCalorimeterHit* hit = (EVENT::RawCalorimeterHit*) rhcol->getElementAt(i);
      if (hit==NULL) continue;
      int cell=hit->getCellID0();
      int PadId=(cell&0x3F0000)>>16;
      int AsicId=(cell&0xFF00)>>8;
      int DifId=cell&0xFF;
#ifdef DEBUG
      cout << "found PadId (0->63)="<<PadId<<" and AsicId="<<AsicId<<endl;
#endif
      stringstream histo, controlh;
      histo<<"/DIF_"<<DifId<<"/Asic_"<<AsicId<<"/Gain_"<<PadGain_;
      if ( InjectedPad_[PadId] ) { //fill scurve
	histo<<"/Scurve";
#ifdef DEBUG
	cout<<"filling scurve : found injected pad"<<endl;
#endif
      }else {  // fill pedestal
	histo<<"/Pedestal";
      }
      controlh<<"/DIF_"<<DifId<<"/Asic_"<<AsicId<<"/Gain_"<<PadGain_<<"/Control";
      histo<<"/Pad_"<<PadId+1;
      //cout<<histo.str()<<endl;
      TH1F* h=(TH1F*)handler_->GetTH1(histo.str());
      TH2F* h2=(TH2F*)handler_->GetTH2(controlh.str());
      if ( h == NULL && InjectedDif_.size()>0 && PadGain_> 0 ) {
	initHistograms(DifId,AsicId);
	h=(TH1F*)handler_->GetTH1(histo.str());
	h2=(TH2F*)handler_->GetTH2(controlh.str());
      } 
      if ( h != NULL && h2 != NULL) {
	h->Fill(LowThresh_);
	std::vector<short>::iterator pads=InjectedPadVector_.begin();
	for ( ; pads !=InjectedPadVector_.end();++pads)
	  h2->Fill(*pads+1,PadId+1);
      } else {
#ifdef DEBUG
	cout << "Could not fill histogram for pad"<<PadId<<endl;
#endif
      }
    } 
#ifdef DEBUG
  cout << "DCScurveMaker exiting processEvent"<<endl;
#endif
  return;
}

void DCScurveMaker::endJob()
{
#ifdef DEBUG
  cout<<"DCScurveMaker --> End of Job"<<endl;
#endif
  int nb=UsedGains_.size();
  if (nb==0) return;
  float avg=0,sg2=0;
  for ( vector<short>::iterator gain_it=UsedGains_.begin(); gain_it != UsedGains_.end(); ++gain_it) { 
    avg+=*gain_it;
    sg2+=(*gain_it)*(*gain_it);
  }
  avg/=nb;
  sg2/=nb;
#ifdef DEBUG
  cout<<"DCScurveMaker --> avg="<<avg<<endl;
#endif

  try {
    gSystem->Exec("mkdir /tmp/ScurveResults");
  } catch(...) { cout <<" result directory exists already"<<endl; }
  stringstream pedgaintit, seuilgaintit;
  for ( std::vector<short>::iterator difit=InjectedDif_.begin();difit!=InjectedDif_.end();difit++){
    pedgaintit<<"/DIF_"<<*difit<<"/EndCalib/PedestalsVSGain";
    seuilgaintit<<"/DIF_"<<*difit<<"/EndCalib/ThresholdsVSGain";
    short gain_min=UsedGains_.at(0); 
    short gain_max= UsedGains_.at(UsedGains_.size()-1);
    TProfile* pedgain=(TProfile*)handler_->BookProfile(pedgaintit.str(), UsedGains_.size(),gain_min-0.5, gain_max+0.5,-1e+32,1e+32);
    TProfile* seuilgain=(TProfile*)handler_->BookProfile(seuilgaintit.str(), UsedGains_.size(),gain_min-0.5, gain_max+0.5,-1e+32,1e+32);
    pedgain->Sumw2();
    seuilgain->Sumw2();
    seuilgain->GetXaxis()->SetTitle("Gain");
    pedgain->GetXaxis()->SetTitle("Gain");
    seuilgain->GetYaxis()->SetTitle("<Threshold>");
    pedgain->GetYaxis()->SetTitle("<Threshold>");
    pedgain->SetLineWidth(2);
    seuilgain->SetLineWidth(2);
    pedgain->SetLineColor(kRed);
    seuilgain->SetLineColor(kBlue);

    for ( vector<short>::iterator gain_it=UsedGains_.begin(); gain_it != UsedGains_.end(); ++gain_it) {
      stringstream scurves, pedestals;
      pedestals<<"/DIF_"<<*difit<<"/EndCalib/Gain_"<<*gain_it<<"/Pedestals";
      scurves<<"/DIF_"<<*difit<<"/EndCalib/Gain_"<<*gain_it<<"/Thresholds";
      TH1F *pedh=(TH1F*)handler_->BookTH1(pedestals.str(),380-LowThresh_,LowThresh_,380);
      pedh->GetXaxis()->SetTitle("All pedestals");
      pedh->SetFillColor(kRed);
      TH1F *scurveh=(TH1F*)handler_->BookTH1(scurves.str(),380-LowThresh_,LowThresh_,380);
      scurveh->GetXaxis()->SetTitle("All thresholds");
      scurveh->SetFillColor(kBlue);
      for ( int asic=1; asic<=48;++asic){
	stringstream pedasics, scasics; 
	scasics<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/EndCalib/Gain_"<<*gain_it<<"/Thresholds";
	pedasics<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/EndCalib/Gain_"<<*gain_it<<"/Pedestals";
	TH1F* seuilasich=(TH1F*)handler_->BookTH1(scasics.str(),64,0.5,64.5);     
	TH1F* pedasich=(TH1F*)handler_->BookTH1(pedasics.str(),64,0.5,64.5);
	seuilasich->GetXaxis()->SetTitle("Pad");
	seuilasich->GetYaxis()->SetTitle("Threshold");
	pedasich->GetXaxis()->SetTitle("Pad");
	pedasich->GetYaxis()->SetTitle("Pedestal");
	seuilasich->SetLineColor(kBlue);
	seuilasich->SetLineWidth(2);
	pedasich->SetLineColor(kRed);
	pedasich->SetLineWidth(2);
      }
      for ( int asic=1; asic<=48; asic++) {
	string startdir="/Users/muriel/ILC/TraitementM2/Config/oldConfigs/";
	//string startdir="/data/online/config/";
	stringstream filename, headstring;
	if ( *difit < 10 ) filename<<"HR2_FT10100"<<*difit<<"_"<<asic<<".cfg";
	else filename<<"HR2_FT1010"<<*difit<<"_"<<asic<<".cfg";
	headstring<<"head -33 "<<startdir<<filename.str()<<"> /tmp/ScurveResults/"<<filename.str();
	gSystem->Exec(headstring.str().c_str());
	std::map<short,vector<short> >::iterator padmap=ActivePads_.find(asic);
	if ( padmap != ActivePads_.end()) {
	  for ( vector<short>::iterator pad=padmap->second.begin();pad!=padmap->second.end();pad++) {
	    stringstream padstring;
	    vector<float> scurveThreshold;
	    vector<float> pedestalThreshold;
	    float avs=0,avp=0,ssg=0,spg=0,ss2=0,sp2=0;
	    for ( vector<short>::iterator gain_it=UsedGains_.begin(); gain_it != UsedGains_.end(); ++gain_it) {
	      //piedestaux
	      stringstream pedasics, pedestals, pedestal;
	      pedestals<<"/DIF_"<<*difit<<"/EndCalib/Gain_"<<*gain_it<<"/Pedestals";
	      pedasics<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/EndCalib/Gain_"<<*gain_it<<"/Pedestals";
	      pedestal<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/Gain_"<<*gain_it<<"/Pedestal/Pad_"<<*pad+1;
	      TH1F *pedh=(TH1F*)handler_->GetTH1(pedestals.str());
	      TH1F *h=(TH1F*)handler_->GetTH1(pedestal.str());
	      TH1F* pedasich=(TH1F*)handler_->GetTH1(pedasics.str());
	      if ( h!=NULL) {
		float hmax=h->GetBinContent(4);
		if (hmax!=0) {
		  h->Scale(1./hmax);
		  h->GetYaxis()->SetTitle("Efficiency");
		  h->GetYaxis()->SetRangeUser(0.,1.05);
		  int nent=4;
		  while ( h->GetBinContent(nent)>0.5 && nent < h->GetNbinsX())nent++;
		  pedestal<<"_Derivative";
		  float width=h->GetXaxis()->GetBinWidth(1);
		  TH1F* hder=(TH1F*)handler_->BookTH1(pedestal.str(),h->GetNbinsX()-1,h->GetXaxis()->GetXmin()+width/2,h->GetXaxis()->GetXmax()-width/2);
		  hder->SetLineColor(h->GetLineColor());
		  hder->SetLineWidth(h->GetLineWidth());
		  for (int bin=1; bin<h->GetNbinsX();++bin){
		    float delta=(h->GetBinContent(bin)-h->GetBinContent(bin+1))/width;
		    hder->SetBinContent(bin,delta);
		  }
		  hder->Fit("gaus","Q");
		  float seuil=h->GetXaxis()->GetBinLowEdge(nent);
		  pedestalThreshold.push_back(seuil);
		  pedasich->Fill(*pad+1,seuil);
		  pedh->Fill(seuil);
		  pedgain->Fill(*gain_it,seuil);
		  avp+=seuil;
		  spg+=seuil*(*gain_it);
		  sp2+=seuil*seuil;
		}
	      }
	      //scurves
	      stringstream scurve,scurves, scasics; 	    
	      scurves<<"/DIF_"<<*difit<<"/EndCalib/Gain_"<<*gain_it<<"/Thresholds";	    
	      scasics<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/EndCalib/Gain_"<<*gain_it<<"/Thresholds";
	      scurve<<"/DIF_"<<*difit<<"/Asic_"<<asic<<"/Gain_"<<*gain_it<<"/Scurve/Pad_"<<*pad+1;
	      h=(TH1F*)handler_->GetTH1(scurve.str());
	      if ( h==NULL) { cout<<"missing scurve for pad "<<*pad+1<<endl;continue ;}
	      TH1F *scurveh=(TH1F*)handler_->GetTH1(scurves.str());	    
	      TH1F* seuilasich=(TH1F*)handler_->GetTH1(scasics.str());     
	    
	      float hmax=h->GetBinContent(4);
	      if (hmax!=0) {
		h->Scale(1./hmax);
		h->GetYaxis()->SetTitle("Efficiency");
		h->GetYaxis()->SetRangeUser(0.,1.05);
		int nent=4;
		while ( h->GetBinContent(nent)>0.5 && nent < h->GetNbinsX())nent++;
		scurve<<"_Derivative";
		float width=h->GetXaxis()->GetBinWidth(1);
		TH1F* hder=(TH1F*)handler_->BookTH1(scurve.str(),h->GetNbinsX()-1,h->GetXaxis()->GetXmin()+width/2,h->GetXaxis()->GetXmax()-width/2);
		hder->SetLineColor(h->GetLineColor());
		hder->SetLineWidth(h->GetLineWidth());
		for (int bin=1; bin<h->GetNbinsX();++bin){
		  float delta=(h->GetBinContent(bin)-h->GetBinContent(bin+1))/width;
		  hder->SetBinContent(bin,delta);
		}
		hder->Fit("gaus","Q");
		
		float seuil=h->GetXaxis()->GetBinLowEdge(nent);
		scurveThreshold.push_back(seuil);
		scurveh->Fill(seuil);
		seuilasich->Fill(*pad+1,seuil);
		seuilgain->Fill(*gain_it,seuil);
		avs+=seuil;
		ssg+=seuil*(*gain_it);
		ss2+=seuil*seuil;
	      }
	    }
	    avp/=nb;
	    avs/=nb;
	    ss2=ss2/nb-avs*avs;
	    sp2=sp2/nb-avp*avp;
	    ssg=ssg/nb-avs*avg;
	    spg=spg/nb-avp*avg;
	    float a=0;
	    if ( ss2 >0 ) a=ssg/ss2;
	    if ( avs < avp+10 ) {
	      cout<< "Asic "<<asic<<" pad "<<*pad+1<<" is dead  seuil="<<avs<<" ped="<<avp<<endl;
	      padstring<<"echo \"Preamp_Gain"<<*pad<<" = 0\" >>/tmp/ScurveResults/"<<filename.str(); 
	    }else{
	      int outgain;
	      if ( hrtype_==1 ) outgain= a*(195-avs)+avg+0.5;
	      else outgain= a*(135-avs)+avg+0.5;
	      cout <<  "Asic "<<asic<<" pad "<<*pad+1<<" has gain "<<outgain<<endl;
	      padstring<<"echo \"Preamp_Gain"<<*pad<<" = "<<outgain<<"\">>/tmp/ScurveResults/"<<filename.str(); 
	    }
	    gSystem->Exec(padstring.str().c_str());
	  }
	}
      } 
    }
  }
#ifdef DEBUG
      cout << "DCScurveMaker exiting endJob"<<endl;
#endif
}

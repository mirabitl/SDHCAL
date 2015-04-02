#include "onedifhandler.h"


using namespace Difhw;
onedifhandler::onedifhandler( std::string objectname,
			      std::string serveraddress,
			      yami::agent* clientagent) : object_name(objectname),server_address(serveraddress),client_agent(clientagent)
{
  yami::parameters params;
  const std::string update_object_name =
    "update_handler";
  params.set_string("destination_object", update_object_name);
    
  s =new Difhw::Statemachine((*client_agent),server_address,object_name);
}

  

void onedifhandler::operator()(yami::incoming_message & message)
{
  const yami::parameters & content =message.get_parameters();
  d.read(content);
  char* buf=d.Payload.data();int* ibuf=(int*) buf;
  std::cout << "received update: " << d.Difid<<" "<<d.Gtc <<" "<<d.Payload.size()<<"->"<<ibuf[0]<<" "<<ibuf[1]<<" "<<ibuf[2]<< std::endl;
}
void onedifhandler::Destroy(){s->Destroy(dst);}
void onedifhandler::Stop(){s->Stop(dst);}

void onedifhandler::Start(){s->Start(dst);}
void onedifhandler::Scan(){s->Scan(Res);}
void onedifhandler::Initialise(){s->Initialise(Res,dst);}
void onedifhandler::Configure(int32_t reg,std::string dbs)
{
  cf.Trigger=reg;
  cf.Dbstate=dbs;
  s->Registerdb(cf,dst);
}
void onedifhandler::LoadSlowControl()
{
  s->Loadslowcontrol(dst);
}
void onedifhandler::Print()
{
  for (int i=0;i<dst.Status.size();i++)
    std::cout<<std::hex<<dst.Status[i]<<std::dec<<" "<<dst.Debug[i]<<std::endl;
}
void onedifhandler::Subscribe()
{
  yami::parameters params;
  const std::string update_object_name =
    "update_handler";
  params.set_string("destination_object", update_object_name);
  client_agent->register_object(update_object_name, *this);
  for (std::vector<int>::iterator it=Res.Diflist.begin();it!=Res.Diflist.end();it++)
    {
      std::cout<<"Subscirbing " <<(*it)<<std::endl;
      std::stringstream ss;
      ss<<"/DIFSERVER/DIF"<<(*it)<<"/DATA";
      client_agent->send_one_way(server_address,
				 ss.str(), "subscribe", params);
    } 
}

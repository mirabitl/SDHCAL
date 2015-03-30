#include "browser.h"

using namespace Difhw;
browser::browser(std::string nameserver_address, yami::agent* clientagent) :name_server_address(nameserver_address),client_agent(clientagent)
{
}
      
    

void browser::QueryList()
{
  yami::parameters list_params;
  
  list_params.set_string("object", "unessai");
    

  std::auto_ptr<yami::outgoing_message> ns_query1( client_agent->send(name_server_address, "names", "list", list_params));

  ns_query1->wait_for_completion();

  if (ns_query1->get_state() != yami::replied)
    {
      std::cout << "error: "
                << ns_query1->get_exception_msg() << std::endl;
      
      return;
    }
  
  const yami::parameters & list_reply =ns_query1->get_reply();


  std::size_t size_ = list_reply.get_string_array_length("location");
  vnames.resize(size_);
  vlocs.resize(size_);
  for (std::size_t i_ = 0; i_ != size_; ++i_)
    {
      vnames[i_] = list_reply.get_string_in_array("object", i_);
      vlocs[i_] = list_reply.get_string_in_array("location", i_);
      std::cout<<"Service "<<vnames[i_]<<" found @ "<<vlocs[i_]<<std::endl;

    }

}

  

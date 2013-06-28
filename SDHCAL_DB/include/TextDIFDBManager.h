#ifndef _TextDIFDBManager_H
#define _TextDIFDBManager_H

#include "DIFDBManager.h"
#include <sstream>
#include <iostream>
#include <fstream>

class TextDIFDBManager : public DIFDBManager
{
public:
  TextDIFDBManager(std::string sdifs,std::string directory);
  virtual ~TextDIFDBManager(){;}
  virtual void initialize();
  virtual uint32_t LoadDIFHardrocV2Parameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v);
  virtual uint32_t LoadDIFMicrorocParameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v);
  virtual void LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v);
  virtual uint32_t LoadAsicParameters(){;}

private:
  std::string theFilePath_;
};
#endif


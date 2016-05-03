#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>
 

//Structure recevant la sortie de LibCurl
extern "C"
{
struct CurlBufferStruct
{
  char* buffer;
  size_t size;
};

static size_t CurlCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
  size_t realsize = size * nmemb;
  struct CurlBufferStruct* mem = (struct CurlBufferStruct*) data;
  mem->buffer = (char*)realloc(mem->buffer, mem->size + realsize + 1);
 
  if ( mem->buffer )
  {
    memcpy(&(mem->buffer[mem->size]), ptr, realsize );
    mem->size += realsize;
    mem->buffer[ mem->size ] = 0;
  }
 
  return realsize;
}
 
 
//Lecture de la page web
  char* CurlQuery(char* AddURL,char* Chaine)
  {
  curl_global_init(CURL_GLOBAL_ALL);
  
  CURL *myHandle;
  CURLcode result;
  struct CurlBufferStruct LectureLC;
  LectureLC.buffer = NULL;
  LectureLC.size = 0;
 
  myHandle = curl_easy_init();
  curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, CurlCallback);
  curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void*)&LectureLC);
  curl_easy_setopt(myHandle, CURLOPT_URL, AddURL);
  result = curl_easy_perform(myHandle);  //voir la doc pour une gestion minimal des erreurs
  curl_easy_cleanup(myHandle);
 
  if(result!=0) LectureLC.size=1;

  strcpy(Chaine, LectureLC.buffer);  strcat(Chaine,"\0");
  if(LectureLC.buffer) free(LectureLC.buffer);
 
  return Chaine;
  }
};

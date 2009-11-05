/**
 * @file   urlget.h
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Sun Jul 26 02:44:36 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef __urlget_h__
#define __urlget_h__

#include <string>
#include <vector>
#include <utility>

#ifdef _WIN32
#ifdef _WIN32 URLGET_EXPORTS
#define URLGET_API __declspec(dllexport)
#else
#define URLGET_API __declspec(dllimport)
#endif
#else
#define URLGET_API
#endif

class URLGET_API UrlGet 
{     

public:
     typedef std::pair<std::string, std::string> FormVariablePair;
     typedef std::vector<FormVariablePair> FormVariables;
     
     UrlGet();
     virtual ~UrlGet();

     bool GetUrl(const std::string& url);
     bool PostUrl(const std::string& url, const FormVariables& formVariables);
     
     void* Realloc(void* ptr, size_t size);

     // Callback must be declared static, otherwise it won't link...
     size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb);

     void Print();

     std::string GetDoc() const;
     
public:
     // Public member vars
     char* m_pBuffer;
     size_t m_Size;

}; // class UrlGet

#endif // __urlget_h__





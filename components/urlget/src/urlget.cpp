/**
 * @file   urlget.cpp
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Sun Jul 26 02:39:05 2009
 * 
 * @brief  
 * 
 * 
 */

#include "urlget.h"

#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
#include "curlpp/Exception.hpp"

#include <iostream>

#define MAX_FILE_LENGTH 20000

UrlGet::UrlGet()
{
    this->m_pBuffer = NULL;
    this->m_pBuffer = (char*) malloc(MAX_FILE_LENGTH * sizeof(char));
    this->m_Size = 0;
};

UrlGet::~UrlGet()
{
    if (this->m_pBuffer)
        free(this->m_pBuffer);
};

void* UrlGet::Realloc(void* ptr, size_t size)
{
    if(ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
};
 
// Callback must be declared static, otherwise it won't link...
size_t UrlGet::WriteMemoryCallback(char* ptr, size_t size, size_t nmemb)
{
    // Calculate the real size of the incoming buffer
    size_t realsize = size * nmemb;

    // (Re)Allocate memory for the buffer
    m_pBuffer = (char*) Realloc(m_pBuffer, m_Size + realsize + 1);

    // Test if Buffer is initialized correctly & copy memory
    if (m_pBuffer == NULL) {
        realsize = 0;
    }
    
    memcpy(&(m_pBuffer[m_Size]), ptr, realsize);
    m_Size += realsize;
    m_pBuffer[m_Size] = 0;
    
    // return the real size of the buffer...
    return realsize;
};

std::string UrlGet::GetDoc() const
{
    return std::string(m_pBuffer);
}

void UrlGet::Print() 
{
    std::cout << "Size: " << m_Size << std::endl;
    std::cout << "Content: " << std::endl << m_pBuffer << std::endl;
}

bool UrlGet::GetUrl(const std::string& url)
{
    bool bOK = false;
    
    try {
        cURLpp::Cleanup cleaner;
        cURLpp::Easy request;
    
        // Set the writer callback to enable cURL 
        // to write result in a memory area
        cURLpp::Types::WriteFunctionFunctor functor(this, &UrlGet::WriteMemoryCallback);
        cURLpp::Options::WriteFunction *test = new cURLpp::Options::WriteFunction(functor);
        request.setOpt(test);
    
        // Setting the URL to retrive.
        request.setOpt(new cURLpp::Options::Url(url.c_str()));
        request.setOpt(new cURLpp::Options::Verbose(false));
        request.perform();

        //Print();

        bOK = true;
    }
    catch ( cURLpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
    }
    catch ( cURLpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
    }

    return bOK;
    
}

bool UrlGet::PostUrl(const std::string& url, const FormVariables& formVariables)
{
    bool bOK = false;
    
    try {
        cURLpp::Cleanup cleaner;
        cURLpp::Easy request;
    
        // Set the writer callback to enable cURL 
        // to write result in a memory area
        cURLpp::Types::WriteFunctionFunctor functor(this, &UrlGet::WriteMemoryCallback);
        cURLpp::Options::WriteFunction *test = new cURLpp::Options::WriteFunction(functor);
        request.setOpt(test);
    
        // Setting the URL to retrive.
        request.setOpt(new cURLpp::Options::Url(url.c_str()));
        request.setOpt(new cURLpp::Options::Verbose(false));

        {
            // Forms takes ownership of pointers!
            curlpp::Forms formParts;

            for ( size_t i = 0 ; i < formVariables.size() ; i++ ){
                const FormVariablePair& fvp = formVariables[i];
                std::string strName = fvp.first;
                std::string strValue = fvp.second;
                
                formParts.push_back(new curlpp::FormParts::Content(strName.c_str(), strValue.c_str()));
            }
            
            request.setOpt(new curlpp::options::HttpPost(formParts)); 
        }

        request.perform();

        //Print();

        bOK = true;
    }
    catch ( cURLpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
    }
    catch ( cURLpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
    }

    return bOK;
    
}

 

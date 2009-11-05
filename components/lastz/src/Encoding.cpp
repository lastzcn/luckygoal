
#include "Encoding.h"
#define LIBICONV_STATIC
#define LIBCHARSET_STATIC
extern "C"{
#include <iconv.h>
}
#include <string>

lastz::Mutex lastz::Encoding::m_mutex;

char g_encodingBuf[1024 * 1024 * 10];

using namespace lastz;

int Encoding::ConvertEncoding(const char* tocode, const char* fromcode, char*strout,
                           const char* Text,int TextLen)
{
    lastz::ScopedLock lock(m_mutex);
    
    int length = 0;

    size_t inbytesleft = TextLen;
    size_t outmaxbytes = TextLen * 4;
    size_t outbytesleft = outmaxbytes;
    iconv_t ic = iconv_open(tocode, fromcode);
    if ( ic == (iconv_t)(-1) ) return 0;

#ifdef _WIN32
    iconv(ic, &Text, &inbytesleft, &strout, &outbytesleft);
#else
    // char szText[1025];
    // char* tmpText = szText;
    // if ( TextLen > 1024 ){
    //     tmpText = new char[TextLen + 1];
    //     //memcpy(tmpText, Text, TextLen);
    //     strcpy(tmpText, Text);        
    // } else {
    //     //memcpy(tmpText, Text, TextLen);
    //     strcpy(tmpText, Text);
    // }

    //iconv(ic, &tmpText, &inbytesleft, &strout, &outbytesleft);

    memcpy(g_encodingBuf, Text, TextLen);
    char* tmpText = g_encodingBuf;
    iconv(ic, &tmpText, &inbytesleft, &strout, &outbytesleft);    
#endif
            
    iconv_close(ic);

#ifndef _WIN32
    // if ( TextLen > 1024 )
    //     delete tmpText;
#endif

    length = outmaxbytes - outbytesleft;

    return length;
}

std::string Encoding::UTF8ToGB(const std::string& str){
    streambuf sBuf;
    UTF8ToGB(sBuf, buffer(str, str.length()));
    std::string strValue = sBuf.data().str();
    return strValue;
}

std::string Encoding::GBToUTF8(const std::string& str){
    streambuf sBuf;
    GBToUTF8(sBuf, buffer(str, str.length()));
    std::string strValue = sBuf.data().str();
    return strValue;

    // std::string toCode ="utf-8";
    // std::string fromCode = "gb18030";
    
    // size_t srcSize = str.length();
    // if ( srcSize > 0 ){
    //     size_t dstSize = srcSize * 30;
    //     char* destBuf = new char[dstSize];

    //     int nConvertedLen = ConvertEncoding(toCode.c_str(), fromCode.c_str(), destBuf, str.c_str(), srcSize);

    //     destBuf[nConvertedLen] = 0;
    //     std::string strRet = destBuf;
    //     delete destBuf;
    //     return strRet;
    // } else {
    //     return std::string("");
    // }
}

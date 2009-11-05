#ifndef __LASTZ_ENCODING_H__
#define __LASTZ_ENCODING_H__

#include "lastz.h"
#include "streambuf.h"
#include "Thread.h"

namespace lastz{

     class LASTZ_API Encoding{
     public:
 
    
        static int ConvertEncoding(const char* tocode, const char* fromcode, char*strout,
                                   const char* Text,int TextLen);

        // GB18030 -> UCS-2
        static int GBToUnicode(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-2", "gb18030", strout, Text, TextLen);
        }

        // UCS-2 -> GB18030
        static int UnicodeToGB(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("gb18030", "ucs-2", strout, Text, TextLen);
        }

        // GB18030 -> UTF-8
        static int GBToUTF8(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("utf-8", "gb18030", strout, Text, TextLen);
        }

        // UTF-8 -> GB18030
        static int UTF8ToGB(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("gb18030", "utf-8", strout, Text, TextLen);
        }

        // GB18030 -> UCS-4
        static int GBToUCS4(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-4", "gb18030", strout, Text, TextLen);
        }

        // UCS-4 -> GB18030
        static int UCS4ToGB(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("gb18030", "ucs-4", strout, Text, TextLen);
        }

        // UCS-2 -> UTF-8
        static int UnicodeToUTF8(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("utf-8", "ucs-2", strout, Text, TextLen);
        }

        // UTF-8 -> UCS-2
        static int UTF8ToUnicode(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-2", "utf-8", strout, Text, TextLen);
        }

        // UCS-4 -> UTF-8
        static int UCS4ToUTF8(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("utf-8", "ucs-4", strout, Text, TextLen);
        }

        // UTF-8 -> UCS-4
        static int UTF8ToUCS4(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-4", "utf-8", strout, Text, TextLen);
        }

        // UCS-2 -> UCS-4
        static int UnicodeToUCS4(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-4", "ucs-2", strout, Text, TextLen);
        }

        // UCS-4 -> UCS-2
        static int UCS4ToUnicode(char* strout, const char* Text, int TextLen){
            return ConvertEncoding("ucs-2", "ucs-4", strout, Text, TextLen);
        }

        static void convertEncodingAppend(streambuf& sBuf, const const_byte_buffer& cBuffer, const std::string& fromCode, const std::string& toCode, size_t maxScale = 4){
            size_t srcSize = cBuffer.size();
            if ( srcSize > 0 ){
                size_t dstSize = srcSize * maxScale;
                mutable_byte_buffer mBuffer = sBuf.prepare(dstSize);
                int nConvertedLen = ConvertEncoding(toCode.c_str(), fromCode.c_str(), mBuffer.data(), cBuffer.data(), cBuffer.size());
                sBuf.commit(nConvertedLen);
            }
        }

        // GB18030 -> UTF-8
        static void GBToUTF8(streambuf& sBuf, const const_byte_buffer& cBuffer){
            convertEncodingAppend(sBuf, cBuffer, "gb18030", "utf-8");
        }

        // UTF-8 -> GB18030
        static void UTF8ToGB(streambuf& sBuf, const const_byte_buffer& cBuffer){
            convertEncodingAppend(sBuf, cBuffer, "utf-8", "gb18030");
        }

        // GB18030 -> UCS-2
        static void GBToUnicode(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "gb18030", "ucs-2");
        }

        // UCS-2 -> GB18030
        static void UnicodeToGB(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-2", "gb18030");
        }

        // GB18030 -> UCS-4
        static void GBToUCS4(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "gb18030", "ucs-4", 4);
        }

        // UCS-4 -> GB18030
        static void UCS4ToGB(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-4", "gb18030", 4);
        }

        // UTF-8 -> UCS-2
        static void UTF8ToUnicode(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "utf-8", "ucs-2");
        }

        // UCS-2 -> UTF-8
        static void UnicodeToUTF8(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-2", "utf-8");
        }

        // UTF-8 -> UCS-4
        static void UTF8ToUCS4(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "utf-8", "ucs-4");
        }

        // UCS-4 -> UTF-8
        static void UCS4ToUTF8(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-4", "utf-8");
        }

        // UCS-4 -> UCS-2
        static void UCS4ToUnicode(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-4", "ucs-2");
        }

        // UCS-2 -> UCS-4
        static void UnicodeToUCS4(streambuf& sBuf, const const_byte_buffer& cBuffer){
			convertEncodingAppend(sBuf, cBuffer, "ucs-2", "ucs-4");
        }

        static std::string UTF8ToGB(const std::string& str);
        static std::string GBToUTF8(const std::string& str);

        static std::string UnicodeToGB(const std::string& str){
             streambuf sBuf;
             UnicodeToGB(sBuf, buffer(str, str.length()));
             std::string strValue = sBuf.data().str();
             return strValue;
        }

        static std::string GBToUnicode(const std::string& str){
             streambuf sBuf;
             GBToUnicode(sBuf, buffer(str, str.length()));
             std::string strValue = sBuf.data().str();
             return strValue;
        }

        static std::string UnicodeToUTF8(const std::string& str){
             streambuf sBuf;
             UnicodeToUTF8(sBuf, buffer(str, str.length()));
             std::string strValue = sBuf.data().str();
             return strValue;
        }

        static std::string UTF8ToUnicode(const std::string& str){
             streambuf sBuf;
             UTF8ToUnicode(sBuf, buffer(str, str.length()));
             std::string strValue = sBuf.data().str();
             return strValue;
        }
        
        static lastz::Mutex m_mutex;
        
    }; // class Encoding
} // namespace lastz

#ifdef _WIN32
#define NString(str) lastz::Encoding::UTF8ToGB(str)
#define UString(str) lastz::Encoding::GBToUTF8(str)
#else
#define NString(str) str
#define UString(str) str
#endif

#endif

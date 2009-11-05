/**
 * @file   sample1.cpp
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Sun Jul 26 06:16:45 2009
 * 
 * @brief  
 * 
 * 
 */

#include "urlget.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cerr << "urlgetsample: Wrong number of arguments" << std::endl 
                  << "urlgetsample: Usage: urlgetsample url" 
                  << std::endl;
        return EXIT_FAILURE;
    }
    char *url = argv[1];
    
    UrlGet ug;
    ug.GetUrl(url);
    
    return 0;
}

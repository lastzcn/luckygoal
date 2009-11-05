// noncopyable.h

#ifndef __LASTZ_NONCOPYABLE_H__
#define __LASTZ_NONCOPYABLE_H__

namespace lastz{
    
    class noncopyable{
    protected:
        noncopyable() {}
        ~noncopyable() {}
    private:  
        noncopyable( const noncopyable& );
        const noncopyable& operator=( const noncopyable& );
    };

}

#endif // __LASTZ_NONCOPYABLE_H__

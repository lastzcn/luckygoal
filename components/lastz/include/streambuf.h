#ifndef __LASTZ_STREAMBUF_H__
#define __LASTZ_STREAMBUF_H__

#include "lastz.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <streambuf>
#include <vector>
#include <iostream>
#include <stdio.h>
//#include <boost/config.hpp>
//#include <boost/noncopyable.hpp>
#include "noncopyable.h"
#include "buffer.h"

#ifdef max
#undef max
#endif

namespace lastz{
    template <typename Allocator = std::allocator<char> >
    class basic_streambuf
        : public std::streambuf
//          ,private noncopyable
    {
    public:
        /// Construct a buffer with a specified maximum size.
        explicit basic_streambuf(
//             std::size_t max_size = (std::numeric_limits<std::size_t>::max)(),
//#ifdef _WIN32
//             std::size_t max_size = (std::numeric_limits<std::size_t>::max)(),
//#else
            size_t max_size = std::numeric_limits<size_t>::max(),
//#endif
            const Allocator& allocator = Allocator())
            : max_size_(max_size),
              buffer_(allocator)
            {
                std::size_t pend = (std::min<std::size_t>)(max_size_, buffer_delta);
                buffer_.resize((std::max<std::size_t>)(pend, 1));
                setg(&buffer_[0], &buffer_[0], &buffer_[0]);
                setp(&buffer_[0], &buffer_[0] + pend);
            }

        void reset(){
			std::size_t pend = (std::min<std::size_t>)(max_size_, buffer_delta);
			buffer_.resize((std::max<std::size_t>)(pend, 1));
            resetg();
            resetp();
        }
        
        void resetg(){
            setg(&buffer_[0], &buffer_[0], &buffer_[0]);
        }
        
        void resetp(){
            setp(&buffer_[0], &buffer_[0]);
        }
        
                
        /// Return the size of the get area in characters.
        std::size_t size() const{
            return pptr() - gptr();
        }

        /// Get a list of buffers that represents the get area.
        const_byte_buffer data() const{
            return lastz::buffer(
                lastz::const_buffer(gptr(),(pptr() - gptr()) * sizeof(char_type))
                );
        }

        std::string str() const{
            const_byte_buffer cBuffer = data();
            return std::string(cBuffer.data(), cBuffer.size());
        }

        /// Get a list of buffers that represents the put area, with the given size.
        mutable_byte_buffer prepare(std::size_t size){
            reserve(size);
            return lastz::buffer(
                lastz::mutable_buffer(pptr(), size * sizeof(char_type))
                );
        }

        /// Move the start of the put area by the specified number of characters.
        void commit(std::size_t n){
            if (pptr() + n > epptr())
                n = epptr() - pptr();
            pbump((int)n);
        }

        /// Move the start of the get area by the specified number of characters.
        void consume(std::size_t n){
            while (n > 0){
                sbumpc();
                --n;
            }
        }

        void append(const const_byte_buffer& cBuffer){
            size_t nSize = cBuffer.size();
            if ( nSize > 0 ){
                mutable_byte_buffer mBuffer = prepare(nSize);
                memcpy(mBuffer.data(), cBuffer.data(), nSize);
                commit(nSize);
            }
        }
         
		inline char encodeChar(char ch){
			if ( ch <= 9 )
				return ch + '0';
			else
				return ch - 10 + 'A';
		}

		inline void encodeChars(char ch, char& ch0, char& ch1){
			char c0 = (ch >> 4) & 0x0F;
			char c1 = ch & 0x0F;
			ch0 = encodeChar(c0);
			ch1 = encodeChar(c1);
		}

		void encodeAppend(const const_byte_buffer& cBuffer){
			size_t nSize = cBuffer.size() * 2;
			mutable_byte_buffer mBuffer = prepare(nSize);
			char* pBuf = mBuffer.data();
			const char* pSrcBuf = cBuffer.data();
			for ( size_t i = 0 ; i < cBuffer.size() ; i++ ){
				char ch = pSrcBuf[i];
				char ch0 = 0;
				char ch1 = 0;
				encodeChars(ch, ch0, ch1);

				pBuf[i * 2] = ch0;
				pBuf[i * 2 + 1] = ch1;
			}
			commit(nSize);
		}

		inline char decodeChar(char ch){
			if ( ch <= '9' )
				return ch - '0';
			else
				return ch - 'A' + 10;
		}

		inline char decodeChars(char c0, char c1){
			char ch0 = decodeChar(c0);
			char ch1 = decodeChar(c1);
			return ch0 << 4 | ch1;
		}

		void decodeAppend(const const_byte_buffer& cBuffer){
			size_t nSize = cBuffer.size() / 2;
			mutable_byte_buffer mBuffer = prepare(nSize);
			char* pBuf = mBuffer.data();
			const char* pSrcBuf = cBuffer.data();
			for ( size_t i = 0 ; i < nSize ; i++ ){
				char ch0 = pSrcBuf[i * 2];
				char ch1 = pSrcBuf[i * 2 + 1];
				char ch = decodeChars(ch0, ch1);
				pBuf[i] = ch;
			}
			commit(nSize);
		}

	protected:
        enum { buffer_delta = 128 };

        int_type underflow(){
            if (gptr() < pptr()){
                setg(&buffer_[0], gptr(), pptr());
                return traits_type::to_int_type(*gptr());
            }else{
                return traits_type::eof();
            }
        }

        int_type overflow(int_type c){
            if (!traits_type::eq_int_type(c, traits_type::eof())){
                if (pptr() == epptr()){
                    std::size_t buffer_size = pptr() - gptr();
                    if (buffer_size < max_size_ && max_size_ - buffer_size < buffer_delta){
                        reserve(max_size_ - buffer_size);
                    }else{
                        reserve(buffer_delta);
                    }
                }

                *pptr() = traits_type::to_char_type(c);
                pbump(1);
                return c;
            }

            return traits_type::not_eof(c);
        }

        void reserve(std::size_t n){
            // Get current stream positions as offsets.
            std::size_t gnext = gptr() - &buffer_[0];
            std::size_t gend = egptr() - &buffer_[0];
            std::size_t pnext = pptr() - &buffer_[0];
            std::size_t pend = epptr() - &buffer_[0];

            // Check if there is already enough space in the put area.
            if (n <= pend - pnext) return;

            // Shift existing contents of get area to start of buffer.
            if (gnext > 0){
                std::rotate(&buffer_[0], &buffer_[0] + gnext, &buffer_[0] + pend);
                gend -= gnext;
                pnext -= gnext;
            }

            // Ensure buffer is large enough to hold at least the specified size.
            if (n > pend - pnext){
//????                if (n <= max_size_ & pnext <= max_size_ - n){
                if (n <= max_size_ && pnext <= max_size_ - n){
                    buffer_.resize((std::max<std::size_t>)(pnext + n, 1));
                }else{
                    throw std::length_error("asio::streambuf too long");
                }
            }

            // Update stream positions.
            setg(&buffer_[0], &buffer_[0], &buffer_[0] + gend);
            setp(&buffer_[0] + pnext, &buffer_[0] + pnext + n);
        }

    private:
        std::size_t max_size_;
        std::vector<char_type, Allocator> buffer_;
    };

    //typedef basic_streambuf<> streambuf;
	class streambuf : public basic_streambuf<>{
	};
}

#endif // __LASTZ_STREAMBUF_H__

/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * Copyright © 2008-2013 Last.fm Limited
 *
 * This file is part of libmoost.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MOOST_NAGIOS_NSCA_CRYPTO_HPP__
#define MOOST_NAGIOS_NSCA_CRYPTO_HPP__

#include <string>
#include <stdexcept>

#ifdef HAVE_LIBMCRYPT
#include <mcrypt.h>
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "nsca_common.hpp"
#include "nsca_data_packet.hpp"

namespace moost{ namespace nagios{

#ifdef WIN32
   inline void bzero(void * p, size_t const size)
   {
      memset(p, 0, size);
   }
#endif

   class nsca_crypto: boost::noncopyable
   {
   private:
      struct crypt_instance{
         char transmitted_iv[nsca_const::TRANSMITTED_IV_SIZE];
#ifdef HAVE_LIBMCRYPT
         MCRYPT td;
         std::vector<char> key;
         std::vector<char> IV;
         char block_buffer;
         int blocksize;
         size_t keysize;
         char *mcrypt_algorithm;
         char *mcrypt_mode;
#endif
      };

      typedef boost::scoped_ptr< crypt_instance > crypt_instance_ptr;

   public:
      nsca_crypto(
         char const * received_iv,
         int const method,
         std::string const & password
         )
         : method_(method)
         , password_(password)
         , CIptr_(new crypt_instance)
      {
         encrypt_init(password_.c_str(), method, received_iv, CIptr_);
      }

      ~nsca_crypto()
      {
         encrypt_cleanup(method_, CIptr_);
      }

      void encrypt(nsca_data_packet & packet) const
      {
         encrypt_buffer(
            reinterpret_cast<char *>(&packet), sizeof(packet),
            password_.c_str(), method_, CIptr_);
      }

      void decrypt(nsca_data_packet & packet) const
      {
         decrypt_buffer(
            reinterpret_cast<char *>(&packet), sizeof(packet),
            password_.c_str(), method_, CIptr_);
      }

   private:
      //---> NASTY CODE ALERT
      // [ricky 7/14/2011] hoiked (and cleaned up a tad) from nsca_send
      //
#ifdef HAVE_LIBMCRYPT
         void encrypt_init(char const *password,int encryption_method,char const * received_iv, crypt_instance_ptr &CI) const
#else // shut up gcc moaning that password and encryption_method are not used
         void encrypt_init(char const *,int encryption_method,char const * received_iv, crypt_instance_ptr &CI) const
#endif
         {

         /* server generates IV used for encryption */
         if(received_iv==NULL)
            throw std::runtime_error("initialization vector not defined");

         /* client recieves IV from server */
         memcpy(CI->transmitted_iv,received_iv,nsca_const::TRANSMITTED_IV_SIZE);

#ifdef HAVE_LIBMCRYPT
         CI->blocksize=1; /* block size = 1 byte w/ CFB mode */
         CI->keysize=7; /* default to 56 bit key length */
         CI->mcrypt_mode="cfb"; /* CFB = 8-bit cipher-feedback mode */
         CI->mcrypt_algorithm="unknown";
#endif

         /* XOR or no encryption */
         if(encryption_method==nsca_encryption_method::ENCRYPT_NONE || encryption_method==nsca_encryption_method::ENCRYPT_XOR)
            return;

#ifdef HAVE_LIBMCRYPT

         /* get the name of the mcrypt encryption algorithm to use */
         switch(encryption_method){
   case nsca_encryption_method::ENCRYPT_DES:
      CI->mcrypt_algorithm=MCRYPT_DES;
      break;
   case nsca_encryption_method::ENCRYPT_3DES:
      CI->mcrypt_algorithm=MCRYPT_3DES;
      break;
   case nsca_encryption_method::ENCRYPT_CAST128:
      CI->mcrypt_algorithm=MCRYPT_CAST_128;
      break;
   case nsca_encryption_method::ENCRYPT_CAST256:
      CI->mcrypt_algorithm=MCRYPT_CAST_256;
      break;
   case nsca_encryption_method::ENCRYPT_XTEA:
      CI->mcrypt_algorithm=MCRYPT_XTEA;
      break;
   case nsca_encryption_method::ENCRYPT_3WAY:
      CI->mcrypt_algorithm=MCRYPT_3WAY;
      break;
   case nsca_encryption_method::ENCRYPT_BLOWFISH:
      CI->mcrypt_algorithm=MCRYPT_BLOWFISH;
      break;
   case nsca_encryption_method::ENCRYPT_TWOFISH:
      CI->mcrypt_algorithm=MCRYPT_TWOFISH;
      break;
   case nsca_encryption_method::ENCRYPT_LOKI97:
      CI->mcrypt_algorithm=MCRYPT_LOKI97;
      break;
   case nsca_encryption_method::ENCRYPT_RC2:
      CI->mcrypt_algorithm=MCRYPT_RC2;
      break;
   case nsca_encryption_method::ENCRYPT_ARCFOUR:
      CI->mcrypt_algorithm=MCRYPT_ARCFOUR;
      break;
   case nsca_encryption_method::ENCRYPT_RIJNDAEL128:
      CI->mcrypt_algorithm=MCRYPT_RIJNDAEL_128;
      break;
   case nsca_encryption_method::ENCRYPT_RIJNDAEL192:
      CI->mcrypt_algorithm=MCRYPT_RIJNDAEL_192;
      break;
   case nsca_encryption_method::ENCRYPT_RIJNDAEL256:
      CI->mcrypt_algorithm=MCRYPT_RIJNDAEL_256;
      break;
   case nsca_encryption_method::ENCRYPT_WAKE:
      CI->mcrypt_algorithm=MCRYPT_WAKE;
      break;
   case nsca_encryption_method::ENCRYPT_SERPENT:
      CI->mcrypt_algorithm=MCRYPT_SERPENT;
      break;
   case nsca_encryption_method::ENCRYPT_ENIGMA:
      CI->mcrypt_algorithm=MCRYPT_ENIGMA;
      break;
   case nsca_encryption_method::ENCRYPT_GOST:
      CI->mcrypt_algorithm=MCRYPT_GOST;
      break;
   case nsca_encryption_method::ENCRYPT_SAFER64:
      CI->mcrypt_algorithm=MCRYPT_SAFER_SK64;
      break;
   case nsca_encryption_method::ENCRYPT_SAFER128:
      CI->mcrypt_algorithm=MCRYPT_SAFER_SK128;
      break;
   case nsca_encryption_method::ENCRYPT_SAFERPLUS:
      CI->mcrypt_algorithm=MCRYPT_SAFERPLUS;
      break;

   default:
      CI->mcrypt_algorithm="unknown";
      break;
         }

         /* open encryption module */
         if((CI->td=mcrypt_module_open(CI->mcrypt_algorithm,NULL,CI->mcrypt_mode,NULL))==MCRYPT_FAILED){
            throw std::runtime_error("Could not open mcrypt algorithm");
         }

         /* determine size of IV buffer for this algorithm */
         size_t iv_size=mcrypt_enc_get_iv_size(CI->td);
         if(iv_size>nsca_const::TRANSMITTED_IV_SIZE){
            throw std::runtime_error("IV size for crypto algorithm exceeds limits");
         }

         CI->IV.resize(iv_size);

         /* fill IV buffer with first bytes of IV that is going to be used to crypt (determined by server) */
         for(size_t i=0;i<iv_size;i++)
            CI->IV[i]=CI->transmitted_iv[i];

         /* get maximum key size for this algorithm */
         CI->keysize=mcrypt_enc_get_key_size(CI->td);

         CI->key.resize(CI->keysize);

         bzero(&CI->key[0],CI->keysize);

         if(CI->keysize<strlen(password))
            strncpy(&CI->key[0],password,CI->keysize);
         else
            strncpy(&CI->key[0],password,strlen(password));

         /* initialize encryption buffers */
         mcrypt_generic_init(CI->td,&CI->key[0],CI->keysize,&CI->IV[0]);

#endif
      }



      /* encryption routine cleanup */
#ifdef HAVE_LIBMCRYPT
      void encrypt_cleanup(int encryption_method, crypt_instance_ptr const & CI)
#else // shut up gcc moaning encryption_method is not used
      void encrypt_cleanup(int, crypt_instance_ptr const & CI)
#endif
      {

         /* no crypt instance */
         if(!CI)
            return;

#ifdef HAVE_LIBMCRYPT
         /* mcrypt cleanup */
         if(encryption_method!=nsca_encryption_method::ENCRYPT_NONE && encryption_method!=nsca_encryption_method::ENCRYPT_XOR){
            mcrypt_generic_end(CI->td);
         }
#endif
      }

      /* encrypt a buffer */
      void encrypt_buffer(char *buffer,size_t buffer_size, char const *password, int encryption_method, crypt_instance_ptr const & CI) const
      {
         size_t x;
         size_t y;
         size_t password_length;

         /* no crypt instance */
         if(!CI)
            return;

         /* no encryption */
         if(encryption_method==nsca_encryption_method::ENCRYPT_NONE)
            return;

         /* simple XOR "encryption" - not meant for any real security, just obfuscates data, but its fast... */
         else if(encryption_method==nsca_encryption_method::ENCRYPT_XOR){

            /* rotate over IV we received from the server... */
            for(y=0,x=0;y<buffer_size;y++,x++){

               /* keep rotating over IV */
               if(x>=nsca_const::TRANSMITTED_IV_SIZE)
                  x=0;

               buffer[y]^=CI->transmitted_iv[x];
            }

            /* rotate over password... */
            password_length=strlen(password);
            for(y=0,x=0;y<buffer_size;y++,x++){

               /* keep rotating over password */
               if(x>=password_length)
                  x=0;

               buffer[y]^=password[x];
            }

            return;
         }

#ifdef HAVE_LIBMCRYPT
         /* use mcrypt routines */
         else{

            /* encrypt each byte of buffer, one byte at a time (CFB mode) */
            for(x=0;x<buffer_size;x++)
               mcrypt_generic(CI->td,&buffer[x],1);
         }
#endif

         return;
      }


      /* decrypt a buffer */
      void decrypt_buffer(char *buffer,size_t buffer_size, char const*password, int encryption_method, crypt_instance_ptr const &CI) const
      {

         /* no crypt instance */
         if(CI==NULL)
            return;

         /* no encryption */
         if(encryption_method==nsca_encryption_method::ENCRYPT_NONE)
            return;

         /* XOR "decryption" is the same as encryption */
         else if(encryption_method==nsca_encryption_method::ENCRYPT_XOR)
            encrypt_buffer(buffer,buffer_size,password,encryption_method,CI);

#ifdef HAVE_LIBMCRYPT
         /* use mcrypt routines */
         else{

            /* encrypt each byte of buffer, one byte at a time (CFB mode) */
            for(size_t x=0;x<buffer_size;x++)
               mdecrypt_generic(CI->td,&buffer[x],1);
         }
#endif

         return;
      }
      // ---<

      int method_;
      std::string password_;
      crypt_instance_ptr CIptr_;
   };

}}

#endif

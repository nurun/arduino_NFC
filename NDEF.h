/**************************************************************************/
/*! 
	@file     PN532_I2C.h
	@author   Odopod Inc.
	@license  BSD
*/
/**************************************************************************/

#ifndef __NDEF_INCLUDED__
#define __NDEF_INCLUDED__

//#include <avr/pgmspace.h>

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE                 (0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT          (0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT         (0x02)
#define NDEF_URIPREFIX_HTTP                 (0x03)
#define NDEF_URIPREFIX_HTTPS                (0x04)
#define NDEF_URIPREFIX_TEL                  (0x05)
#define NDEF_URIPREFIX_MAILTO               (0x06)
#define NDEF_URIPREFIX_FTP_ANONAT           (0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT           (0x08)
#define NDEF_URIPREFIX_FTPS                 (0x09)
#define NDEF_URIPREFIX_SFTP                 (0x0A)
#define NDEF_URIPREFIX_SMB                  (0x0B)
#define NDEF_URIPREFIX_NFS                  (0x0C)
#define NDEF_URIPREFIX_FTP                  (0x0D)
#define NDEF_URIPREFIX_DAV                  (0x0E)
#define NDEF_URIPREFIX_NEWS                 (0x0F)
#define NDEF_URIPREFIX_TELNET               (0x10)
#define NDEF_URIPREFIX_IMAP                 (0x11)
#define NDEF_URIPREFIX_RTSP                 (0x12)
#define NDEF_URIPREFIX_URN                  (0x13)
#define NDEF_URIPREFIX_POP                  (0x14)
#define NDEF_URIPREFIX_SIP                  (0x15)
#define NDEF_URIPREFIX_SIPS                 (0x16)
#define NDEF_URIPREFIX_TFTP                 (0x17)
#define NDEF_URIPREFIX_BTSPP                (0x18)
#define NDEF_URIPREFIX_BTL2CAP              (0x19)
#define NDEF_URIPREFIX_BTGOEP               (0x1A)
#define NDEF_URIPREFIX_TCPOBEX              (0x1B)
#define NDEF_URIPREFIX_IRDAOBEX             (0x1C)
#define NDEF_URIPREFIX_FILE                 (0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID           (0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG          (0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT          (0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW          (0x21)
#define NDEF_URIPREFIX_URN_EPC              (0x22)
#define NDEF_URIPREFIX_URN_NFC              (0x23)

#define NDEF_TYPE_MIME                      (02)
#define NDEF_TYPE_URI                       (85)
#define NDEF_TYPE_TEXT                      (84)

#define NDEF_WELL_KNOWN_RECORD              (0x01)
#define NDEF_MIME_TYPE_RECORD               (0x02)

#define NDEF_BUFFER_SIZE 224
//#define DEBUG

struct FOUND_MESSAGE{
    int type;
    char * format;
    uint8_t * payload;
};

class NDEF{
  public:
    NDEF();
	FOUND_MESSAGE decode_message(uint8_t * msg);
	uint8_t	encode_URI(uint8_t uriPrefix, uint8_t * msg);
    uint8_t encode_TEXT(uint8_t * lang, uint8_t * msg);
    uint8_t encode_MIME(uint8_t * mimetype, uint8_t * data, uint8_t len);
	
  private:
    uint8_t encode_record_header(bool mb, bool me, bool cf, bool sr, bool il, uint8_t tnf);
        
//    char * get_type_description(uint8_t b);
    char * get_uri_prefix(uint8_t b);
    bool parse_uri(uint8_t * payload, int payload_len, char * uri);
    bool parse_text(uint8_t * payload, int payload_len, char * lang, char * text);
};


#endif
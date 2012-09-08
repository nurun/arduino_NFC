#include "NDEF.h"

NDEF::NDEF(){
    
}

/**
 * Parse the actual NDEF message and call specific handlers for dealing with
 * a particular type of NDEF message.
 *
 * @param IN msg  The NDEF message
 * @return             whether or not the parsing succeeds
 */
char * NDEF::decode_message(uint8_t * msg) {
    int offset = 2;
    
    bool me;
    do {
        bool mb = (*(msg + offset) & 0x80) == 0x80;        /* Message Begin */
        me = (*(msg + offset) & 0x40) == 0x40;				/* Message End */
        bool cf = (*(msg + offset) & 0x20) == 0x20;        /* Chunk Flag */
        bool sr = (*(msg + offset) & 0x10) == 0x10;        /* Short Record */
        bool il = (*(msg + offset) & 0x08) == 0x08;        /* ID Length Present */
//        char * typeNameFormat = get_type_description(*(msg + offset) & 0x07);
        uint8_t tn = (*(msg + offset) & 0x07);
        offset++;
        
#ifdef NDEFDEBUG
		Serial.print("MB:"); Serial.println(mb, DEC);
		Serial.print("ME:"); Serial.println(me, DEC);
		Serial.print("CF:"); Serial.println(cf, DEC);
		Serial.print("SR:"); Serial.println(sr, DEC);
		Serial.print("IL:"); Serial.println(il, DEC);
        Serial.print("TNF:"); Serial.println(tn, HEX);
//		Serial.println(typeNameFormat);
#endif
        if (cf) {
            Serial.println("chunk flag not supported yet");
            return false;
        }
        
        int typeLength = *(msg + offset);
        offset++;
        
        int payloadLength;
        if (sr) {
            payloadLength = *(msg + offset);
            payloadLength = (payloadLength < 0) ? payloadLength + 256 : payloadLength;
            offset++;
        } else {
            offset += 4;
        }
        
        int idLength = 0;
        if (il) {
            idLength = *(msg + offset);
            offset++;
        }
        
        uint8_t type = *(msg + offset);
		
        offset += typeLength;
        
        if (il) {
            offset += idLength;
        }
#ifdef DEBUG
        Serial.print("TYPE "); Serial.println((char)type);
#endif
        
        memcpy(msg, msg + offset, payloadLength);
        offset += payloadLength;
        char lang [2];
        char text [NDEF_BUFFER_SIZE];
        char uri [NDEF_BUFFER_SIZE];
        switch (type) {
            case NDEF_TYPE_URL:
                if(parse_uri(msg, payloadLength, uri)){
                    Serial.print("uri: "); Serial.println(uri);
                    return uri;
                }else{
                    Serial.println("err");
                }
                break;
            case NDEF_TYPE_TEXT:
                if(parse_text(msg, payloadLength, lang, text)) {
                    Serial.print("lang: "); Serial.println(lang);
                    Serial.print("text: "); Serial.println(text);
                } else {
                    Serial.println("err");
                }

                break;
            case NDEF_TYPE_SMART_POSTER:
//              Serial.println("Found Smart Poster - Begin");
//              parse_ndef_message(payload);
//              Serial.println("Smart Poster - End");
                break;
            default:
                Serial.print("err, NDEF type: "); Serial.println((char)type);
                return false;
                break;
        }
    } while (!me);              /* as long as this is not the last record */
    
    Serial.println("");
    
    return "";
}

/**
 * encodes the NDEF message attaches the proper formatted header and terminating character
 *
 * @param type          supports: NDEF_TYPE_URL or NDEF_TYPE_TEXT
 * @param msg           the payload
 * @param uriPrefix     optional if you are using a URI prefix
 * @return              length of the encoded message
 */



uint8_t	NDEF::encode_URL(uint8_t uriPrefix, uint8_t * msg){
    uint8_t len = strlen((char *)msg);
    uint8_t payload_head[7] = {0x03, len+5, 0xD1, 0x01, len+1, 0x55, uriPrefix};
    const uint8_t term[1] ={0xFE};

    memmove(msg+7, msg, len);
    memcpy(msg+0, payload_head, 7);
    memcpy(msg + len + 7, term , 1);
#ifdef DEBUG
    for (uint8_t i = 0 ; i < len + 8; i++) {
        Serial.print(msg[i], HEX);Serial.print(" ");
    }
    Serial.println("");
#endif    
    
    return len + 8;
    
}
uint8_t NDEF::encode_TEXT(uint8_t * lang, uint8_t * msg){
    uint8_t len = strlen((char *)msg);

    uint8_t payload_head[8] = {0x03, len+5, 0xD1, 0x01, len+2, 0x54, lang[0], lang[1]};
    const uint8_t term[1] ={0xFE};
    
    memmove(msg+8, msg, len);
    memcpy(msg, payload_head, 8);
    memcpy(msg + len + 8, term , 1);
#ifdef DEBUG
    for (uint8_t i = 0 ; i < len + 9; i++) {
        Serial.print(msg[i], HEX);Serial.print(" ");
    }
    Serial.println("");
#endif
    
    return len + 9;
    
}





/**
 * Extracts the NDEF message from a MIFARE Ultralight tag.
 *
 * @param IN  mtD       the pointer to the mifareul_tag struct
 * @param OUT msg  returned NDEF message
 * @param OUT tlv_len   length of the returned NDEF message
 * @return              whether or not the msg was successfully
 *                      filled
 */

//bool NdefDecode::get_ndef_message_from_dump(uint8_t* dump, uint8_t * msg)
//{
//    int tlv_len = 0;
//    // First byte is TLV type
//    // We are looking for  "NDEF Message TLV" (0x03)
//    if (dump[0] == 0x03) {
//        tlv_len = dump[1];
//        memcpy(msg, dump+2 , tlv_len);
//        Serial.println("NDEF message found in dump.");
//    } else {
//        Serial.println("NDEF message TLV not found");
//        return 0;
//    }
//    return 1;
//}

/**
 * Convert TNF bits to Type (used for debug only). This is specified in
 * section 3.2.6 "TNF (Type Name Format)" of "NFC Data Exchange Format
 * (NDEF) Technical Specification".
 *
 * (we don't really need this)
 *
 * @param IN  b  the value of the last 3 bits of the NDEF record header
 * @return       the human readable type of the NDEF record
 */
//char * NDEF::get_type_description(uint8_t b){
//    switch (b) {
//        case 0x00:
//            return "Empty";
//        case 0x01:
//            return "NFC Forum well-known type NFC RTD";
//        case 0x02:
//            return "Media-type as defined in RFC 2046 RFC 2046";
//        case 0x03:
//            return "Absolute URI as defined in RFC 3986 RFC 3986";
//        case 0x04:
//            return "NFC Forum external type NFC RTD";
//        case 0x05:
//            return "Unknown";
//        case 0x06:
//            return "Unchanged";
//        case 0x07:
//            return "Reserved";
//        default:
//            return "Invalid TNF byte!";
//    }
//}

/**
 * Convert type of URI to the actual URI prefix to be used in conjunction
 * with the URI stored in the NDEF record itself. This is specified in section
 * 3.2.2 "URI Identifier Code" of "URI Record Type Definition Technical
 * Specification".
 *
 
 
 !! comment out ones you don't want to support to save memory size, they add up to alot! 
 
 
 * @param IN  b  the code of the URI to convert to the actual prefix
 * @return       the URI prefix
 */
char * NDEF::get_uri_prefix(uint8_t b)
{
    /*
     * Section 3.2.2 "URI Identifier Code" of "URI Record Type Definition
     * Technical Specification"
     */
    switch (b) {
        case 0x00:
            return "";
        case 0x01:
            return "http://www.";
        case 0x02:
            return "https://www.";
        case 0x03:
            return "http://";
        case 0x04:
            return "https://";
        case 0x05:
            return "tel:";
        case 0x06:
            return "mailto:";
//        case 0x07:
//            return "ftp://anonymous:anonymous@";
//        case 0x08:
//            return "ftp://ftp.";
//        case 0x09:
//            return "ftps://";
//        case 0x0A:
//            return "sftp://";
//        case 0x0B:
//            return "smb://";
//        case 0x0C:
//            return "nfs://";
//        case 0x0D:
//            return "ftp://";
//        case 0x0E:
//            return "dav://";
//        case 0x0F:
//            return "news:";
//        case 0x10:
//            return "telnet://";
//        case 0x11:
//            return "imap:";
//        case 0x12:
//            return "rtsp://";
//        case 0x13:
//            return "urn:";
//        case 0x14:
//            return "pop:";
//        case 0x15:
//            return "sip:";
//        case 0x16:
//            return "sips:";
//        case 0x17:
//            return "tftp:";
//        case 0x18:
//            return "btspp://";
//        case 0x19:
//            return "btl2cap://";
//        case 0x1A:
//            return "btgoep://";
//        case 0x1B:
//            return "tcpobex://";
//        case 0x1C:
//            return "irdaobex://";
        case 0x1D:
            return "file://";
//        case 0x1E:
//            return "urn:epc:id:";
//        case 0x1F:
//            return "urn:epc:tag:";
//        case 0x20:
//            return "urn:epc:pat:";
//        case 0x21:
//            return "urn:epc:raw:";
//        case 0x22:
//            return "urn:epc:";
//        case 0x23:
//            return "urn:nfc:";
        default:
            return "unknown";
    }
}

/**
 * Concatenates the prefix with the contents of the NDEF URI record.
 *
 * @param IN payload      The NDEF URI payload
 * @param IN payload_len  The length of the NDEF URI payload
 * @return                The full reconstructed URI
 */
boolean NDEF::parse_uri(uint8_t * payload, int payload_len, char * uri ){
	char * prefix = get_uri_prefix(payload[0]);
    int prefix_len = strlen(prefix);
    
    memcpy(uri, prefix, prefix_len);
    memcpy(uri + prefix_len, payload + 1, payload_len - 1);
    *(uri + prefix_len + payload_len - 1) = 0x00;
#ifdef DEBUG
        Serial.println(uri);
#endif 

    return true;
}

/**
 * Concatenates the prefix with the contents of the NDEF URI record.
 *
 * @param IN payload      The NDEF Text Record payload
 * @param IN payload_len  The length of the NDEF Text Record payload
 * @param OUT lang        The IANA language code lenght
 * @param OUT text        The text contained in NDEF Text Record
 * @return                Success or not.
 */
boolean NDEF::parse_text(uint8_t * payload, int payload_len, char * lang, char * text){
    bool utf16_format = ((payload[0] & 0x80) == 0x80);
    bool rfu = ((payload[0] & 0x40) == 0x40);
    if(rfu) {
        Serial.println("ERROR: RFU should be set to zero.");
        return false;
    }
    
    memcpy(lang, payload + 1, 2);
    *(lang + 2) = 0x00;
    
    const int text_len = payload_len - 2;
    
#ifdef DEBUG
        Serial.print("Format: "); Serial.println((char *)utf16_format);
//        p("Format: %s, RFU=%d, IANA language code lenght=%d, text lenght=%d\n", utf16_format ? "UTF-16" : "UTF-8", rfu, lang_len, text_len);
#endif

    memcpy(text, payload + 3, text_len);
    *(text + text_len) = 0x00;
    
    return true;
}



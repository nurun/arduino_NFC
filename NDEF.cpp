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
FOUND_MESSAGE NDEF::decode_message(uint8_t * msg) {
    int offset = 2;
    FOUND_MESSAGE m;

    bool mb = (*(msg + offset) & 0x80) == 0x80;        /* Message Begin */
    bool me = (*(msg + offset) & 0x40) == 0x40;        /* Message End */
    bool cf = (*(msg + offset) & 0x20) == 0x20;        /* Chunk Flag */
    bool sr = (*(msg + offset) & 0x10) == 0x10;        /* Short Record */
    bool il = (*(msg + offset) & 0x08) == 0x08;        /* ID Length Present */
    uint8_t tnf = (*(msg + offset) & 0x07);
    offset++;
        
#ifdef NDEFDEBUG
    Serial.print("MB:"); Serial.println(mb, DEC);
    Serial.print("ME:"); Serial.println(me, DEC);
    Serial.print("CF:"); Serial.println(cf, DEC);
    Serial.print("SR:"); Serial.println(sr, DEC);
    Serial.print("IL:"); Serial.println(il, DEC);
    Serial.print("TNF:"); Serial.println(tnf, HEX);
#endif
    if (cf) {
        Serial.println("chunk flag not supported yet");
        m.type = 0;
        return m;
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
    switch ((int)tnf) {
        case 1:
            //well known record type
            m.type = *(msg + offset);
            
            offset += typeLength;
            
            if (il) {
                offset += idLength;
            }
                
            memcpy(msg, msg + offset, payloadLength);
            offset += payloadLength;
            char lang [2];
            char text [NDEF_BUFFER_SIZE];
            char uri [NDEF_BUFFER_SIZE];
            
            switch (m.type) {
                case NDEF_TYPE_URI:
                    if(parse_uri(msg, payloadLength, uri)){
//                      Serial.print("uri: "); Serial.println(uri);
                        m.format = (char *)(uint8_t)msg[0];
                        m.payload = (uint8_t*)uri;
                    }
                    break;
                case NDEF_TYPE_TEXT:
                    if(parse_text(msg, payloadLength, lang, text)) {
//                      Serial.print("lang: "); Serial.println(lang);
//                      Serial.print("text: "); Serial.println(text);
                        m.format = lang;
                        m.payload = (uint8_t*)text;
                    }
                    break;
                default:
                    Serial.println("err, NDEF type: "); Serial.println((char)m.type);
                    break;
                }
            break;
        case 2:
            //mime type record
            m.type = NDEF_TYPE_MIME;
            
            char mimetype [typeLength-2];
            uint8_t payload [NDEF_BUFFER_SIZE];
            
            memcpy(mimetype, msg+offset, typeLength);
            memcpy(payload, msg +typeLength +offset, payloadLength - typeLength);
                
//            Serial.print("mimetype: "); Serial.println(mimetype);
//            Serial.print("data: "); Serial.println((char*)payload);
            
            m.format = mimetype;
            m.payload = payload;
                
            break;
        default:
            Serial.println("err");
            break;
        }   
        
    return m;
}

/**
 * encodes the NDEF message attaches the proper formatted header and terminating character
 *
 * @param type          supports: NDEF_TYPE_URI or NDEF_TYPE_TEXT
 * @param msg           the payload
 * @param uriPrefix     optional if you are using a URI prefix
 * @return              length of the encoded message
 */



uint8_t	NDEF::encode_URI(uint8_t uriPrefix, uint8_t * msg){
    uint8_t len = strlen((char *)msg);
    
    uint8_t record_header = encode_record_header(1, 1, 0, 1, 0, NDEF_WELL_KNOWN_RECORD);
    
    uint8_t payload_head[7] = {0x03, len+5, record_header, 0x01, len+1, 0x55, uriPrefix};
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

    uint8_t record_header = encode_record_header(1, 1, 0, 1, 0, NDEF_WELL_KNOWN_RECORD);
   
    uint8_t payload_head[9] = {0x03, len+7, record_header, 0x01, len+3, 0x54, 0x02, lang[0], lang[1]};
    const uint8_t term[1] ={0xFE};
    
    memmove(msg + 9, msg, len);
    memcpy(msg, payload_head, 9);
    memcpy(msg + 9 + len, term , 1);
#ifdef DEBUG
    for (uint8_t i = 0 ; i < len + 10; i++) {
        Serial.print(msg[i], HEX);Serial.print(" ");
    }
    Serial.println("");
#endif
    
    return len + 10;
    
}

uint8_t NDEF::encode_MIME(uint8_t * mimetype, uint8_t * data, uint8_t len){
    uint8_t typeLen = strlen((char *) mimetype);
    
    uint8_t record_header = encode_record_header(1, 1, 0, 1, 0, NDEF_MIME_TYPE_RECORD);
    
    uint8_t payload_head[5] = {0x03, len + typeLen + 3, record_header, typeLen, len};
    const uint8_t term[1] ={0xFE};

    memmove(data + 5 + typeLen, data, len);
    memcpy(data, payload_head, 5);
    memcpy(data + 5, mimetype, typeLen);
    memcpy(data + 5 + typeLen + len,  term , 1);
    
    return typeLen + len + 6;
}

uint8_t NDEF::encode_record_header(bool mb, bool me, bool cf, bool sr, bool il, uint8_t tnf){
    uint8_t record_header = 0;
    
    if(mb)
        record_header += 0x80;
    if(me)
        record_header += 0x40;
    if(cf)
        record_header += 0x20;
    if(sr)
        record_header += 0x10;
    if(il)
        record_header += 0x08;
    
    record_header += tnf;

    return record_header;
}


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
        case 0x0B:
            return "smb://";
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
bool NDEF::parse_uri(uint8_t * payload, int payload_len, char * uri ){
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
bool NDEF::parse_text(uint8_t * payload, int payload_len, char * lang, char * text){
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



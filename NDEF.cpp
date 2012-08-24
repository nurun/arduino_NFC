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
char * NDEF::decode_message(char * msg) {
    int offset = 2;
    
    bool me;
    do {
        bool mb = (*(msg + offset) & 0x80) == 0x80;        /* Message Begin */
        me = (*(msg + offset) & 0x40) == 0x40;				/* Message End */
        bool cf = (*(msg + offset) & 0x20) == 0x20;        /* Chunk Flag */
        bool sr = (*(msg + offset) & 0x10) == 0x10;        /* Short Record */
        bool il = (*(msg + offset) & 0x08) == 0x08;        /* ID Length Present */
        char * typeNameFormat = get_type_description(*(msg + offset) & 0x07);
        offset++;
        
#ifdef DEBUG
		Serial.print("MB:"); Serial.println(mb, DEC);
		Serial.print("ME:"); Serial.println(me, DEC);
		Serial.print("CF:"); Serial.println(cf, DEC);
		Serial.print("SR:"); Serial.println(sr, DEC);
		Serial.print("IL:"); Serial.println(il, DEC);
		Serial.println(typeNameFormat);
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
            //FIXME payloadLength = Utils.byteArrayToInt(data, offset);
            offset += 4;
        }
        
        int idLength = 0;
        if (il) {
            idLength = *(msg + offset);
            offset++;
        }
        char *type =  (char*)malloc(typeLength);
        memcpy(type, msg + offset, typeLength);
		
        offset += typeLength;
        
        uint8_t *id;
        if (il) {
            id =  (uint8_t *)malloc(idLength);
            memcpy(id, msg + offset, idLength);
            offset += idLength;
        }
#ifdef DEBUG
		//            p("typeLength=%d payloadLength=%d idLength=%d type=%s\n", typeLength, payloadLength, idLength, type);
#endif
        uint8_t *payload = (uint8_t *)malloc(payloadLength);
        
        memcpy(payload, msg + offset, payloadLength);
        offset += payloadLength;
        
        if (strncmp("U", type, typeLength) == 0) {
            /* handle URI case */
            char *uri = ndef_parse_uri(payload, payloadLength);
            Serial.print("found uri: "); Serial.println(uri);
//        } else if(strncmp("Sp", type, typeLength) == 0) {
//            Serial.println("Found Smart Poster - Begin");
//            parse_ndef_message(payload);
//            Serial.println("Smart Poster - End");
        } else if(strncmp("T", type, typeLength) == 0) {
            char *lang;
            char *text;
            if(ndef_parse_text(payload, payloadLength, &lang, &text)) {
                Serial.print("found text lang: "); Serial.println(lang);
                Serial.print("text: "); Serial.println(text);
            } else {
                Serial.println("Unable to parse one Text Record.");
            }
        } else {
            Serial.print("unsupported NDEF record type: "); Serial.println((char *)type);
            return false;
        }
    } while (!me);              /* as long as this is not the last record */
    
    Serial.println("");
    
    return type;
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
 * @param IN  b  the value of the last 3 bits of the NDEF record header
 * @return       the human readable type of the NDEF record
 */
char * NDEF::get_ndef_type_description(uint8_t b){
    switch (b) {
        case 0x00:
            return "Empty";
        case 0x01:
            return "NFC Forum well-known type NFC RTD";
        case 0x02:
            return "Media-type as defined in RFC 2046 RFC 2046";
        case 0x03:
            return "Absolute URI as defined in RFC 3986 RFC 3986";
        case 0x04:
            return "NFC Forum external type NFC RTD";
        case 0x05:
            return "Unknown";
        case 0x06:
            return "Unchanged";
        case 0x07:
            return "Reserved";
        default:
            return "Invalid TNF byte!";
    }
}

/**
 * Convert type of URI to the actual URI prefix to be used in conjunction
 * with the URI stored in the NDEF record itself. This is specified in section
 * 3.2.2 "URI Identifier Code" of "URI Record Type Definition Technical
 * Specification".
 *
 * @param IN  b  the code of the URI to convert to the actual prefix
 * @return       the URI prefix
 */
char *NDEF::get_uri_prefix(uint8_t b)
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
        case 0x07:
            return "ftp://anonymous:anonymous@";
        case 0x08:
            return "ftp://ftp.";
        case 0x09:
            return "ftps://";
        case 0x0A:
            return "sftp://";
        case 0x0B:
            return "smb://";
        case 0x0C:
            return "nfs://";
        case 0x0D:
            return "ftp://";
        case 0x0E:
            return "dav://";
        case 0x0F:
            return "news:";
        case 0x10:
            return "telnet://";
        case 0x11:
            return "imap:";
        case 0x12:
            return "rtsp://";
        case 0x13:
            return "urn:";
        case 0x14:
            return "pop:";
        case 0x15:
            return "sip:";
        case 0x16:
            return "sips:";
        case 0x17:
            return "tftp:";
        case 0x18:
            return "btspp://";
        case 0x19:
            return "btl2cap://";
        case 0x1A:
            return "btgoep://";
        case 0x1B:
            return "tcpobex://";
        case 0x1C:
            return "irdaobex://";
        case 0x1D:
            return "file://";
        case 0x1E:
            return "urn:epc:id:";
        case 0x1F:
            return "urn:epc:tag:";
        case 0x20:
            return "urn:epc:pat:";
        case 0x21:
            return "urn:epc:raw:";
        case 0x22:
            return "urn:epc:";
        case 0x23:
            return "urn:nfc:";
        default:
            return "RFU";
    }
}

/**
 * Concatenates the prefix with the contents of the NDEF URI record.
 *
 * @param IN payload      The NDEF URI payload
 * @param IN payload_len  The length of the NDEF URI payload
 * @return                The full reconstructed URI
 */
char * NDEF::parse_uri(uint8_t * payload, int payload_len){
	char *prefix = get_uri_prefix(*payload);
    int prefix_len = strlen(prefix);
    char *url = (char*)malloc(prefix_len + payload_len);
    memcpy(url, prefix, prefix_len);
    memcpy(url + prefix_len, payload + 1, payload_len - 1);
    *(url + prefix_len + payload_len - 1) = 0x00;
#ifdef DEBUG
        Serial.println(url);
#endif 

    return url;
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
char * NDEF::parse_text(uint8_t * payload, int payload_len, char ** lang, char ** text)
{
    //    if (DEBUG) print_hex(payload);
    bool utf16_format = ((payload[0] & 0x80) == 0x80);
    bool rfu = ((payload[0] & 0x40) == 0x40);
    if(rfu) {
        Serial.println("ERROR: RFU should be set to zero.");
        return 0;
    }
    
    int lang_len = payload[0] & 0x3F;
    (*lang) = (char*) malloc(lang_len +1);
    memcpy(*lang, payload + 1, lang_len);
    (*lang)[lang_len] = '\0';
    
    const int text_len = payload_len - lang_len;
    
#ifdef DEBUG
        Serial.print("Format: "); Serial.println((char *)utf16_format);
//        p("Format: %s, RFU=%d, IANA language code lenght=%d, text lenght=%d\n", utf16_format ? "UTF-16" : "UTF-8", rfu, lang_len, text_len);
#endif



    (*text) =  (char*)malloc(text_len+1);
    memcpy(*text, payload + 1 + lang_len, text_len);
    (*text)[text_len] = L'\0';
    
    return *text;
}



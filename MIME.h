
//#define DEBUG 1

class NdefDecode{
  public:
    NdefDecode(void);
    bool get_ndef_message_from_dump(uint8_t* dump, uint8_t * ndef_msg);
    char * type_name_format(uint8_t b);
    char *uri_identifier_code(uint8_t b);
    char *ndef_parse_uri(uint8_t * payload, int payload_len);
    bool ndef_parse_text(uint8_t * payload, int payload_len, char ** lang, char ** text);
    bool parse_ndef_message(uint8_t * ndef_msg);

};
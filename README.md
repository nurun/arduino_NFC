arduino_NFC
===========

Library to support arduino development with NFC readers/writes.

Our goal was to simplify and normalize an API to support reading and writing NDEF tags for URI records, Plain text, or MIME data types, to mifare classic or mifare ultralight NFC tags.

This Library is interchangeable with shields using either I2C or SPI, and includes some utilities and functions based on the https://github.com/adafruit/Adafruit_NFCShield_I2C library, and PN532_SPI library by Seeed Technology Inc http://www.seeedstudio.com/wiki/NFC_Shield

There are 2 examples; read and write which both have alternate functionality commented out to support different options for I2C / SPI or URI / TEXT / MIME. For the most part Classic / Ultralight are interchangeable without code changes. 

Since there are so many options you may run into memory limits. Keeping a small buffer size, and commenting out functions you don't need for your application will help if you run into these limits. 
 
The files are split into 3 different sections (classes): 

The PN532 chip level supports IO bus for the I2C and SPI variants.
The Mifare level supports generic reading and writing to Classic and Ultralight tags.
The NDEF level supports the encoding and decoding of NDEF formatted content. 


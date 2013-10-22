#ifndef _NVMFEAUTURES_H_
#define _NVMFEAUTURES_H_

#define NVMFILE_VERSION    2
#define NVMFILE_MAGIC      0xBE000000L


#define NVM_FEAUTURE_LOOKUPSWITCH (1L<<0)
#define NVM_FEAUTURE_TABLESWITCH  (1L<<1)
#define NVM_FEAUTURE_32BIT        (1L<<2)
#define NVM_FEAUTURE_FLOAT        (1L<<3)
#define NVM_FEAUTURE_ARRAY        (1L<<4)
#define NVM_FEAUTURE_INHERITANCE  (1L<<5)
#define NVM_FEAUTURE_EXTSTACK     (1L<<6)


#define NVM_MAGIC_FEAUTURE (NVMFILE_MAGIC\
                           |NVM_FEAUTURE_LOOKUPSWITCH\
                           |NVM_FEAUTURE_TABLESWITCH\
                           |NVM_FEAUTURE_32BIT\
                           |NVM_FEAUTURE_FLOAT\
                           |NVM_FEAUTURE_ARRAY\
                           |NVM_FEAUTURE_INHERITANCE)


#endif // _NVMFEAUTURES_H_

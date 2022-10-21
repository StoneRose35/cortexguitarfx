#include "audio/waveShaper.h"

#ifndef FLOAT_AUDIO
const WaveShaperDataType waveShaperUnity = {
    .transferFunctionPoints = {
        0x8001, 0x8201, 0x8401, 0x8601, 0x8801, 0x8a01, 0x8c01, 0x8e01, 0x9001, 0x9201, 0x9401, 0x9601, 0x9801, 0x9a01, 0x9c01, 0x9e01, 0xa001, 0xa201, 0xa401, 0xa601, 0xa801, 0xaa01, 0xac01, 0xae01, 0xb001, 0xb201, 0xb401, 0xb601, 0xb801, 0xba01, 0xbc01, 0xbe01, 0xc001, 0xc201, 0xc401, 0xc601, 0xc801, 0xca01, 0xcc01, 0xce01, 0xd001, 0xd201, 0xd401, 0xd601, 0xd801, 0xda01, 0xdc01, 0xde01, 0xe001, 0xe201, 0xe401, 0xe601, 0xe801, 0xea01, 0xec01, 0xee01, 0xf001, 0xf201, 0xf401, 0xf601, 0xf801, 0xfa01, 0xfc01, 0xfe01, 0x0, 0x1ff, 0x3ff, 0x5ff, 0x7ff, 0x9ff, 0xbff, 0xdff, 0xfff, 0x11ff, 0x13ff, 0x15ff, 0x17ff, 0x19ff, 0x1bff, 0x1dff, 0x1fff, 0x21ff, 0x23ff, 0x25ff, 0x27ff, 0x29ff, 0x2bff, 0x2dff, 0x2fff, 0x31ff, 0x33ff, 0x35ff, 0x37ff, 0x39ff, 0x3bff, 0x3dff, 0x3fff, 0x41ff, 0x43ff, 0x45ff, 0x47ff, 0x49ff, 0x4bff, 0x4dff, 0x4fff, 0x51ff, 0x53ff, 0x55ff, 0x57ff, 0x59ff, 0x5bff, 0x5dff, 0x5fff, 0x61ff, 0x63ff, 0x65ff, 0x67ff, 0x69ff, 0x6bff, 0x6dff, 0x6fff, 0x71ff, 0x73ff, 0x75ff, 0x77ff, 0x79ff, 0x7bff, 0x7dff, 
  
        //0x8001, 0x8206, 0x840a, 0x860e, 0x8812, 0x8a16, 0x8c1a, 0x8e1e, 0x9022, 0x9226, 0x942a, 0x962e, 0x9832, 0x9a36, 0x9c3a, 0x9e3e, 0xa042, 0xa246, 0xa44a, 0xa64e, 0xa852, 0xaa56, 0xac5a, 0xae5e, 0xb062, 0xb266, 0xb46a, 0xb66e, 0xb872, 0xba76, 0xbc7a, 0xbe7e, 0xc082, 0xc286, 0xc48a, 0xc68e, 0xc892, 0xca96, 0xcc9a, 0xce9e, 0xd0a2, 0xd2a6, 0xd4aa, 0xd6ae, 0xd8b2, 0xdab6, 0xdcba, 0xdebe, 0xe0c2, 0xe2c6, 0xe4ca, 0xe6ce, 0xe8d2, 0xead6, 0xecda, 0xeede, 0xf0e2, 0xf2e6, 0xf4ea, 0xf6ee, 0xf8f2, 0xfaf6, 0xfcfa, 0xfefe, 0x102, 0x306, 0x50a, 0x70e, 0x912, 0xb16, 0xd1a, 0xf1e, 0x1122, 0x1326, 0x152a, 0x172e, 0x1932, 0x1b36, 0x1d3a, 0x1f3e, 0x2142, 0x2346, 0x254a, 0x274e, 0x2952, 0x2b56, 0x2d5a, 0x2f5e, 0x3162, 0x3366, 0x356a, 0x376e, 0x3972, 0x3b76, 0x3d7a, 0x3f7e, 0x4182, 0x4386, 0x458a, 0x478e, 0x4992, 0x4b96, 0x4d9a, 0x4f9e, 0x51a2, 0x53a6, 0x55aa, 0x57ae, 0x59b2, 0x5bb6, 0x5dba, 0x5fbe, 0x61c2, 0x63c6, 0x65ca, 0x67ce, 0x69d2, 0x6bd6, 0x6dda, 0x6fde, 0x71e2, 0x73e6, 0x75ea, 0x77ee, 0x79f2, 0x7bf6, 0x7dfa, 0x7fff, 
    }
};

const WaveShaperDataType waveShaperDefaultOverdrive = {
    .transferFunctionPoints = {
        0x8001, 0x8028, 0x804e, 0x8075, 0x809b, 0x80c1, 0x80e8, 0x810e, 0x8135, 0x815b, 0x8181, 0x81a8, 0x81ce, 0x81f5, 0x821b, 0x8241, 0x8268, 0x828e, 0x82b5, 0x82db, 0x8301, 0x8328, 0x834e, 0x8375, 0x839b, 0x83c1, 0x83fe, 0x850b, 0x86ef, 0x897a, 0x8c7e, 0x8fcc, 0x9335, 0x969b, 0x9a01, 0x9d68, 0xa0ce, 0xa434, 0xa79b, 0xab01, 0xae68, 0xb1ce, 0xb534, 0xb89b, 0xbc01, 0xbf67, 0xc2ce, 0xc634, 0xc99b, 0xcd01, 0xd067, 0xd3ce, 0xd734, 0xda9a, 0xde01, 0xe167, 0xe4ce, 0xe834, 0xeb9a, 0xef01, 0xf267, 0xf5cd, 0xf934, 0xfc9a, 0x0, 0x366, 0x6cc, 0xa33, 0xd99, 0x10ff, 0x1466, 0x17cc, 0x1b32, 0x1e99, 0x21ff, 0x2566, 0x28cc, 0x2c32, 0x2f99, 0x32ff, 0x3665, 0x39cc, 0x3d32, 0x4099, 0x43ff, 0x4765, 0x4acc, 0x4e32, 0x5198, 0x54ff, 0x5865, 0x5bcc, 0x5f32, 0x6298, 0x65ff, 0x6965, 0x6ccb, 0x7034, 0x7382, 0x7686, 0x7911, 0x7af5, 0x7c02, 0x7c3f, 0x7c65, 0x7c8b, 0x7cb2, 0x7cd8, 0x7cff, 0x7d25, 0x7d4b, 0x7d72, 0x7d98, 0x7dbf, 0x7de5, 0x7e0b, 0x7e32, 0x7e58, 0x7e7f, 0x7ea5, 0x7ecb, 0x7ef2, 0x7f18, 0x7f3f, 0x7f65, 0x7f8b, 0x7fb2, 0x7fd8, 

        //0x8001, 0x8028, 0x804f, 0x8076, 0x809c, 0x80c3, 0x80ea, 0x8110, 0x8137, 0x815e, 0x8185, 0x81ab, 0x81d2, 0x81f9, 0x821f, 0x8246, 0x826d, 0x8293, 0x82ba, 0x82e1, 0x8308, 0x832e, 0x8355, 0x837c, 0x83a2, 0x83c9, 0x8421, 0x8562, 0x8772, 0x8a22, 0x8d41, 0x90a0, 0x9410, 0x977d, 0x9aea, 0x9e58, 0xa1c5, 0xa532, 0xa89f, 0xac0c, 0xaf7a, 0xb2e7, 0xb654, 0xb9c1, 0xbd2f, 0xc09c, 0xc409, 0xc776, 0xcae3, 0xce51, 0xd1be, 0xd52b, 0xd898, 0xdc06, 0xdf73, 0xe2e0, 0xe64d, 0xe9bb, 0xed28, 0xf095, 0xf402, 0xf76f, 0xfadd, 0xfe4a, 0x1b6, 0x523, 0x891, 0xbfe, 0xf6b, 0x12d8, 0x1645, 0x19b3, 0x1d20, 0x208d, 0x23fa, 0x2768, 0x2ad5, 0x2e42, 0x31af, 0x351d, 0x388a, 0x3bf7, 0x3f64, 0x42d1, 0x463f, 0x49ac, 0x4d19, 0x5086, 0x53f4, 0x5761, 0x5ace, 0x5e3b, 0x61a8, 0x6516, 0x6883, 0x6bf0, 0x6f60, 0x72bf, 0x75de, 0x788e, 0x7a9e, 0x7bdf, 0x7c37, 0x7c5e, 0x7c84, 0x7cab, 0x7cd2, 0x7cf8, 0x7d1f, 0x7d46, 0x7d6d, 0x7d93, 0x7dba, 0x7de1, 0x7e07, 0x7e2e, 0x7e55, 0x7e7b, 0x7ea2, 0x7ec9, 0x7ef0, 0x7f16, 0x7f3d, 0x7f64, 0x7f8a, 0x7fb1, 0x7fd8, 0x7fff
    }
};

const WaveShaperDataType waveShaperSoftOverdrive = {
    .transferFunctionPoints = {
        0x8001, 0x8013, 0x8024, 0x8035, 0x804a, 0x8062, 0x8081, 0x80a6, 0x80d5, 0x810e, 0x8153, 0x81a6, 0x8208, 0x827b, 0x8301, 0x839b, 0x844b, 0x8513, 0x85f3, 0x86ee, 0x8805, 0x893b, 0x8a8f, 0x8c05, 0x8d9d, 0x8f5a, 0x913c, 0x9346, 0x9579, 0x97d7, 0x9a61, 0x9d1a, 0xa001, 0xa301, 0xa601, 0xa901, 0xac01, 0xaf01, 0xb201, 0xb501, 0xb801, 0xbb01, 0xbe01, 0xc101, 0xc401, 0xc701, 0xca01, 0xcd01, 0xd001, 0xd301, 0xd601, 0xd901, 0xdc01, 0xdf01, 0xe201, 0xe501, 0xe801, 0xeb01, 0xee01, 0xf101, 0xf401, 0xf701, 0xfa01, 0xfd01, 0x0, 0x2ff, 0x5ff, 0x8ff, 0xbff, 0xeff, 0x11ff, 0x14ff, 0x17ff, 0x1aff, 0x1dff, 0x20ff, 0x23ff, 0x26ff, 0x29ff, 0x2cff, 0x2fff, 0x32ff, 0x35ff, 0x38ff, 0x3bff, 0x3eff, 0x41ff, 0x44ff, 0x47ff, 0x4aff, 0x4dff, 0x50ff, 0x53ff, 0x56ff, 0x59ff, 0x5cff, 0x5fff, 0x62e6, 0x659f, 0x6829, 0x6a87, 0x6cba, 0x6ec4, 0x70a6, 0x7263, 0x73fb, 0x7571, 0x76c5, 0x77fb, 0x7912, 0x7a0d, 0x7aed, 0x7bb5, 0x7c65, 0x7cff, 0x7d85, 0x7df8, 0x7e5a, 0x7ead, 0x7ef2, 0x7f2b, 0x7f5a, 0x7f7f, 0x7f9e, 0x7fb6, 0x7fcb, 0x7fdc, 0x7fed, 

        //0x8001, 0x8013, 0x8024, 0x8036, 0x804b, 0x8063, 0x8082, 0x80a8, 0x80d8, 0x8112, 0x8159, 0x81ae, 0x8212, 0x8288, 0x8311, 0x83af, 0x8463, 0x852f, 0x8615, 0x8716, 0x8834, 0x8971, 0x8acd, 0x8c4c, 0x8dee, 0x8fb6, 0x91a4, 0x93bb, 0x95fb, 0x9868, 0x9b02, 0x9dcb, 0xa0c3, 0xa3c9, 0xa6cf, 0xa9d5, 0xacdb, 0xafe1, 0xb2e7, 0xb5ed, 0xb8f3, 0xbbf9, 0xbeff, 0xc205, 0xc50b, 0xc811, 0xcb17, 0xce1d, 0xd123, 0xd429, 0xd72f, 0xda35, 0xdd3b, 0xe041, 0xe347, 0xe64d, 0xe953, 0xec59, 0xef5f, 0xf265, 0xf56b, 0xf871, 0xfb77, 0xfe7d, 0x183, 0x489, 0x78f, 0xa95, 0xd9b, 0x10a1, 0x13a7, 0x16ad, 0x19b3, 0x1cb9, 0x1fbf, 0x22c5, 0x25cb, 0x28d1, 0x2bd7, 0x2edd, 0x31e3, 0x34e9, 0x37ef, 0x3af5, 0x3dfb, 0x4101, 0x4407, 0x470d, 0x4a13, 0x4d19, 0x501f, 0x5325, 0x562b, 0x5931, 0x5c37, 0x5f3d, 0x6235, 0x64fe, 0x6798, 0x6a05, 0x6c45, 0x6e5c, 0x704a, 0x7212, 0x73b4, 0x7533, 0x768f, 0x77cc, 0x78ea, 0x79eb, 0x7ad1, 0x7b9d, 0x7c51, 0x7cef, 0x7d78, 0x7dee, 0x7e52, 0x7ea7, 0x7eee, 0x7f28, 0x7f58, 0x7f7e, 0x7f9d, 0x7fb5, 0x7fca, 0x7fdc, 0x7fed, 0x7fff
    }
};

const WaveShaperDataType waveShaperDistortion = {
    .transferFunctionPoints = {
        0x8001, 0x8009, 0x8011, 0x8019, 0x8021, 0x8029, 0x8031, 0x8039, 0x8041, 0x8049, 0x8051, 0x8059, 0x8061, 0x8069, 0x8071, 0x8079, 0x8081, 0x8089, 0x8091, 0x8099, 0x80a1, 0x80a9, 0x80b1, 0x80b9, 0x80c1, 0x80c9, 0x80d1, 0x80d9, 0x80e1, 0x80e9, 0x80f1, 0x80f9, 0x8101, 0x8109, 0x8111, 0x8119, 0x8121, 0x8129, 0x8131, 0x8139, 0x8141, 0x8149, 0x8141, 0x813c, 0x816a, 0x81fb, 0x8320, 0x8508, 0x87e3, 0x8be2, 0x9134, 0x980a, 0xa001, 0xa801, 0xb001, 0xb801, 0xc001, 0xc801, 0xd001, 0xd801, 0xe001, 0xe801, 0xf001, 0xf801, 0x0, 0x7ff, 0xfff, 0x17ff, 0x1fff, 0x27ff, 0x2fff, 0x37ff, 0x3fff, 0x47ff, 0x4fff, 0x57ff, 0x5fff, 0x67f6, 0x6ecc, 0x741e, 0x781d, 0x7af8, 0x7ce0, 0x7e05, 0x7e96, 0x7ec4, 0x7ebf, 0x7eb7, 0x7ebf, 0x7ec7, 0x7ecf, 0x7ed7, 0x7edf, 0x7ee7, 0x7eef, 0x7ef7, 0x7eff, 0x7f07, 0x7f0f, 0x7f17, 0x7f1f, 0x7f27, 0x7f2f, 0x7f37, 0x7f3f, 0x7f47, 0x7f4f, 0x7f57, 0x7f5f, 0x7f67, 0x7f6f, 0x7f77, 0x7f7f, 0x7f87, 0x7f8f, 0x7f97, 0x7f9f, 0x7fa7, 0x7faf, 0x7fb7, 0x7fbf, 0x7fc7, 0x7fcf, 0x7fd7, 0x7fdf, 0x7fe7, 0x7fef, 0x7ff7, 
        
        //0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8000, 0x8041, 0x8198, 0x84d3, 0x88bc, 0x8ca5, 0x908e, 0x9477, 0x9860, 0x9c49, 0xa032, 0xa41b, 0xa804, 0xabed, 0xafd7, 0xb3c0, 0xb7a9, 0xbb92, 0xbf7b, 0xc364, 0xc74d, 0xcb36, 0xcf1f, 0xd308, 0xd6f1, 0xdada, 0xdec3, 0xe2ac, 0xe696, 0xea7f, 0xee68, 0xf251, 0xf63a, 0xfa23, 0xfe0c, 0x1f4, 0x5dd, 0x9c6, 0xdaf, 0x1198, 0x1581, 0x196a, 0x1d54, 0x213d, 0x2526, 0x290f, 0x2cf8, 0x30e1, 0x34ca, 0x38b3, 0x3c9c, 0x4085, 0x446e, 0x4857, 0x4c40, 0x5029, 0x5413, 0x57fc, 0x5be5, 0x5fce, 0x63b7, 0x67a0, 0x6b89, 0x6f72, 0x735b, 0x7744, 0x7b2d, 0x7e68, 0x7fbf, 0x8000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff

    }
};

const WaveShaperDataType waveShaperCurvedOverdrive = {
    .transferFunctionPoints = {
        0x8001, 0x80ac, 0x8157, 0x81f3, 0x8277, 0x82e4, 0x833e, 0x8386, 0x83be, 0x83e7, 0x8405, 0x8418, 0x8423, 0x8428, 0x8428, 0x8426, 0x8424, 0x8424, 0x8427, 0x8430, 0x8440, 0x845a, 0x8480, 0x84b3, 0x84f5, 0x8549, 0x85b1, 0x862d, 0x86c2, 0x876f, 0x8838, 0x891e, 0x8a24, 0x8b4a, 0x8c94, 0x8e03, 0x8f9a, 0x9159, 0x9343, 0x955b, 0x97a2, 0x9a1a, 0x9cc4, 0x9fa4, 0xa2bb, 0xa60a, 0xa995, 0xad5c, 0xb163, 0xb5aa, 0xba34, 0xbf02, 0xc401, 0xc901, 0xce01, 0xd301, 0xd801, 0xdd01, 0xe201, 0xe701, 0xec01, 0xf101, 0xf601, 0xfb01, 0x0, 0x4ff, 0x9ff, 0xeff, 0x13ff, 0x18ff, 0x1dff, 0x22ff, 0x27ff, 0x2cff, 0x31ff, 0x36ff, 0x3bff, 0x40fe, 0x45cc, 0x4a56, 0x4e9d, 0x52a4, 0x566b, 0x59f6, 0x5d45, 0x605c, 0x633c, 0x65e6, 0x685e, 0x6aa5, 0x6cbd, 0x6ea7, 0x7066, 0x71fd, 0x736c, 0x74b6, 0x75dc, 0x76e2, 0x77c8, 0x7891, 0x793e, 0x79d3, 0x7a4f, 0x7ab7, 0x7b0b, 0x7b4d, 0x7b80, 0x7ba6, 0x7bc0, 0x7bd0, 0x7bd9, 0x7bdc, 0x7bdc, 0x7bda, 0x7bd8, 0x7bd8, 0x7bdd, 0x7be8, 0x7bfb, 0x7c19, 0x7c42, 0x7c7a, 0x7cc2, 0x7d1c, 0x7d89, 0x7e0d, 0x7ea9, 0x7f54, 

        
        //0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8003, 0x8018, 0x8049, 0x8095, 0x80fc, 0x817e, 0x821b, 0x82d3, 0x83a7, 0x8495, 0x859e, 0x86c3, 0x8803, 0x895d, 0x8ad3, 0x8c64, 0x8e10, 0x8fd7, 0x91b9, 0x93b6, 0x95cf, 0x9802, 0x9a51, 0x9cba, 0x9f3f, 0xa1df, 0xa49a, 0xa76f, 0xaa61, 0xad6d, 0xb094, 0xb3d6, 0xb734, 0xbaac, 0xbe40, 0xc1ee, 0xc5b8, 0xc99d, 0xcd9c, 0xd1a4, 0xd5ac, 0xd9b4, 0xddbc, 0xe1c4, 0xe5cc, 0xe9d4, 0xeddc, 0xf1e4, 0xf5ec, 0xf9f4, 0xfdfc, 0x204, 0x60c, 0xa14, 0xe1c, 0x1224, 0x162c, 0x1a34, 0x1e3c, 0x2244, 0x264c, 0x2a54, 0x2e5c, 0x3264, 0x3663, 0x3a48, 0x3e12, 0x41c0, 0x4554, 0x48cc, 0x4c2a, 0x4f6c, 0x5293, 0x559f, 0x5891, 0x5b66, 0x5e21, 0x60c1, 0x6346, 0x65af, 0x67fe, 0x6a31, 0x6c4a, 0x6e47, 0x7029, 0x71f0, 0x739c, 0x752d, 0x76a3, 0x77fd, 0x793d, 0x7a62, 0x7b6b, 0x7c59, 0x7d2d, 0x7de5, 0x7e82, 0x7f04, 0x7f6b, 0x7fb7, 0x7fe8, 0x7ffd, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff
    }
};

const WaveShaperDataType waveShaperAsymm = {
    .transferFunctionPoints = {
    0x999b, 0x999b, 0x999e, 0x99a6, 0x99b7, 0x99d3, 0x99fd, 0x9a38, 0x9a86, 0x9aeb, 0x9b68, 0x9c02, 0x9cba, 0x9d93, 0x9e91, 0x9fb5, 0xa103, 0xa27d, 0xa427, 0xa602, 0xa801, 0xaa01, 0xac01, 0xae01, 0xb001, 0xb201, 0xb401, 0xb601, 0xb801, 0xba01, 0xbc01, 0xbe01, 0xc001, 0xc201, 0xc401, 0xc601, 0xc801, 0xca01, 0xcc01, 0xce01, 0xd001, 0xd201, 0xd401, 0xd601, 0xd801, 0xda01, 0xdc01, 0xde01, 0xe001, 0xe201, 0xe401, 0xe601, 0xe801, 0xea01, 0xec01, 0xee01, 0xf001, 0xf201, 0xf401, 0xf601, 0xf801, 0xfa01, 0xfc01, 0xfe01, 0x0, 0x1ff, 0x3ff, 0x5ff, 0x7ff, 0x9ff, 0xbff, 0xdff, 0xfff, 0x11ff, 0x13ff, 0x15ff, 0x17ff, 0x19ff, 0x1bff, 0x1dff, 0x1fff, 0x21ff, 0x23d1, 0x2509, 0x25c4, 0x2628, 0x2658, 0x26df, 0x26ff, 0x271f, 0x273f, 0x275f, 0x277f, 0x279f, 0x27bf, 0x27df, 0x27ff, 0x281f, 0x283f, 0x285f, 0x287f, 0x289f, 0x28bf, 0x28df, 0x28ff, 0x291f, 0x293f, 0x295f, 0x297f, 0x299f, 0x29bf, 0x29df, 0x29ff, 0x2a1f, 0x2a3f, 0x2a5f, 0x2a7f, 0x2a9f, 0x2abf, 0x2adf, 0x2aff, 0x2b1f, 0x2b3f, 0x2b5f, 0x2b7f, 0x2b9f, 0x2bbf, 0x2bdf,    
    }
};



void initWaveShaper(WaveShaperDataType * data,const WaveShaperDataType* copyFrom)
{
    for (uint8_t c=0;c<128;c++)
    {
        data->transferFunctionPoints[c]=copyFrom->transferFunctionPoints[c];
    }
}

int16_t waveShaperProcessSample(int16_t sampleIn,WaveShaperDataType* data)
{
    uint32_t indx;
    uint16_t sampleUint;
    uint32_t rem;
    int16_t out;
    sampleUint = 0x8000 + sampleIn;
    indx = ((((uint16_t)sampleUint) & 0xFE00) >> 9);
    rem = sampleUint - (uint16_t)((sampleUint) & 0xFE00);
    if (indx < 127)
    {
        out = data->transferFunctionPoints[indx] + (((int16_t)rem*(data->transferFunctionPoints[indx+1]-data->transferFunctionPoints[indx])) >> 9);
    }
    else
    {
        out = data->transferFunctionPoints[indx];
    }
    return out;
}

#else

const WaveShaperDataType waveShaperUnity = {
    .transferFunctionPoints = {
        -1.000000f, -0.984375f, -0.968750f, -0.953125f, -0.937500f, -0.921875f, -0.906250f, -0.890625f, -0.875000f, -0.859375f, -0.843750f, -0.828125f, -0.812500f, -0.796875f, -0.781250f, -0.765625f, -0.750000f, -0.734375f, -0.718750f, -0.703125f, -0.687500f, -0.671875f, -0.656250f, -0.640625f, -0.625000f, -0.609375f, -0.593750f, -0.578125f, -0.562500f, -0.546875f, -0.531250f, -0.515625f, -0.500000f, -0.484375f, -0.468750f, -0.453125f, -0.437500f, -0.421875f, -0.406250f, -0.390625f, -0.375000f, -0.359375f, -0.343750f, -0.328125f, -0.312500f, -0.296875f, -0.281250f, -0.265625f, -0.250000f, -0.234375f, -0.218750f, -0.203125f, -0.187500f, -0.171875f, -0.156250f, -0.140625f, -0.125000f, -0.109375f, -0.093750f, -0.078125f, -0.062500f, -0.046875f, -0.031250f, -0.015625f, 0.000000f, 0.015625f, 0.031250f, 0.046875f, 0.062500f, 0.078125f, 0.093750f, 0.109375f, 0.125000f, 0.140625f, 0.156250f, 0.171875f, 0.187500f, 0.203125f, 0.218750f, 0.234375f, 0.250000f, 0.265625f, 0.281250f, 0.296875f, 0.312500f, 0.328125f, 0.343750f, 0.359375f, 0.375000f, 0.390625f, 0.406250f, 0.421875f, 0.437500f, 0.453125f, 0.468750f, 0.484375f, 0.500000f, 0.515625f, 0.531250f, 0.546875f, 0.562500f, 0.578125f, 0.593750f, 0.609375f, 0.625000f, 0.640625f, 0.656250f, 0.671875f, 0.687500f, 0.703125f, 0.718750f, 0.734375f, 0.750000f, 0.765625f, 0.781250f, 0.796875f, 0.812500f, 0.828125f, 0.843750f, 0.859375f, 0.875000f, 0.890625f, 0.906250f, 0.921875f, 0.937500f, 0.953125f, 0.968750f, 0.984375f,
    }
};

const WaveShaperDataType waveShaperDefaultOverdrive = {
    .transferFunctionPoints = {
        -1.000000f, -0.998828f, -0.997656f, -0.996484f, -0.995313f, -0.994141f, -0.992969f, -0.991797f, -0.990625f, -0.989453f, -0.988281f, -0.987109f, -0.985938f, -0.984766f, -0.983594f, -0.982422f, -0.981250f, -0.980078f, -0.978906f, -0.977734f, -0.976562f, -0.975391f, -0.974219f, -0.973047f, -0.971875f, -0.970703f, -0.968863f, -0.960640f, -0.945874f, -0.925997f, -0.902438f, -0.876629f, -0.850000f, -0.823437f, -0.796875f, -0.770312f, -0.743750f, -0.717187f, -0.690625f, -0.664062f, -0.637500f, -0.610938f, -0.584375f, -0.557812f, -0.531250f, -0.504687f, -0.478125f, -0.451562f, -0.425000f, -0.398438f, -0.371875f, -0.345312f, -0.318750f, -0.292187f, -0.265625f, -0.239062f, -0.212500f, -0.185938f, -0.159375f, -0.132812f, -0.106250f, -0.079687f, -0.053125f, -0.026562f, 0.000000f, 0.026562f, 0.053125f, 0.079687f, 0.106250f, 0.132812f, 0.159375f, 0.185938f, 0.212500f, 0.239062f, 0.265625f, 0.292187f, 0.318750f, 0.345312f, 0.371875f, 0.398438f, 0.425000f, 0.451562f, 0.478125f, 0.504687f, 0.531250f, 0.557812f, 0.584375f, 0.610938f, 0.637500f, 0.664062f, 0.690625f, 0.717187f, 0.743750f, 0.770312f, 0.796875f, 0.823437f, 0.850000f, 0.876629f, 0.902438f, 0.925997f, 0.945874f, 0.960640f, 0.968863f, 0.970703f, 0.971875f, 0.973047f, 0.974219f, 0.975391f, 0.976562f, 0.977734f, 0.978906f, 0.980078f, 0.981250f, 0.982422f, 0.983594f, 0.984766f, 0.985938f, 0.987109f, 0.988281f, 0.989453f, 0.990625f, 0.991797f, 0.992969f, 0.994141f, 0.995313f, 0.996484f, 0.997656f, 0.998828f, 
    }
};

const WaveShaperDataType waveShaperSoftOverdrive = {
    .transferFunctionPoints = {
        -1.000000f, -0.999479f, -0.998958f, -0.998413f, -0.997792f, -0.997045f, -0.996123f, -0.994977f, -0.993559f, -0.991818f, -0.989706f, -0.987174f, -0.984173f, -0.980653f, -0.976566f, -0.971862f, -0.966493f, -0.960409f, -0.953561f, -0.945900f, -0.937377f, -0.927944f, -0.917549f, -0.906146f, -0.893684f, -0.880115f, -0.865390f, -0.849458f, -0.832273f, -0.813783f, -0.793940f, -0.772696f, -0.750000f, -0.726562f, -0.703125f, -0.679688f, -0.656250f, -0.632812f, -0.609375f, -0.585938f, -0.562500f, -0.539062f, -0.515625f, -0.492188f, -0.468750f, -0.445312f, -0.421875f, -0.398438f, -0.375000f, -0.351562f, -0.328125f, -0.304688f, -0.281250f, -0.257812f, -0.234375f, -0.210938f, -0.187500f, -0.164062f, -0.140625f, -0.117188f, -0.093750f, -0.070312f, -0.046875f, -0.023438f, 0.000000f, 0.023438f, 0.046875f, 0.070312f, 0.093750f, 0.117188f, 0.140625f, 0.164062f, 0.187500f, 0.210938f, 0.234375f, 0.257812f, 0.281250f, 0.304688f, 0.328125f, 0.351562f, 0.375000f, 0.398438f, 0.421875f, 0.445312f, 0.468750f, 0.492188f, 0.515625f, 0.539062f, 0.562500f, 0.585938f, 0.609375f, 0.632812f, 0.656250f, 0.679688f, 0.703125f, 0.726562f, 0.750000f, 0.772696f, 0.793940f, 0.813783f, 0.832273f, 0.849458f, 0.865390f, 0.880115f, 0.893684f, 0.906146f, 0.917549f, 0.927944f, 0.937377f, 0.945900f, 0.953561f, 0.960409f, 0.966493f, 0.971862f, 0.976566f, 0.980653f, 0.984173f, 0.987174f, 0.989706f, 0.991818f, 0.993559f, 0.994977f, 0.996123f, 0.997045f, 0.997792f, 0.998413f, 0.998958f, 0.999479f, 
    }
};

const WaveShaperDataType waveShaperDistortion = {
    .transferFunctionPoints = {
        -1.000000f, -0.999756f, -0.999512f, -0.999268f, -0.999023f, -0.998779f, -0.998535f, -0.998291f, -0.998047f, -0.997803f, -0.997559f, -0.997314f, -0.997070f, -0.996826f, -0.996582f, -0.996338f, -0.996094f, -0.995850f, -0.995605f, -0.995361f, -0.995117f, -0.994873f, -0.994629f, -0.994385f, -0.994141f, -0.993896f, -0.993652f, -0.993408f, -0.993164f, -0.992920f, -0.992676f, -0.992432f, -0.992188f, -0.991943f, -0.991699f, -0.991455f, -0.991211f, -0.990967f, -0.990723f, -0.990479f, -0.990234f, -0.989991f, -0.990245f, -0.990403f, -0.989000f, -0.984567f, -0.975640f, -0.960750f, -0.938431f, -0.907216f, -0.865638f, -0.812230f, -0.750000f, -0.687500f, -0.625000f, -0.562500f, -0.500000f, -0.437500f, -0.375000f, -0.312500f, -0.250000f, -0.187500f, -0.125000f, -0.062500f, 0.000000f, 0.062500f, 0.125000f, 0.187500f, 0.250000f, 0.312500f, 0.375000f, 0.437500f, 0.500000f, 0.562500f, 0.625000f, 0.687500f, 0.750000f, 0.812230f, 0.865638f, 0.907216f, 0.938431f, 0.960750f, 0.975640f, 0.984567f, 0.989000f, 0.990403f, 0.990245f, 0.989991f, 0.990234f, 0.990479f, 0.990723f, 0.990967f, 0.991211f, 0.991455f, 0.991699f, 0.991943f, 0.992188f, 0.992432f, 0.992676f, 0.992920f, 0.993164f, 0.993408f, 0.993652f, 0.993896f, 0.994141f, 0.994385f, 0.994629f, 0.994873f, 0.995117f, 0.995361f, 0.995605f, 0.995850f, 0.996094f, 0.996338f, 0.996582f, 0.996826f, 0.997070f, 0.997314f, 0.997559f, 0.997803f, 0.998047f, 0.998291f, 0.998535f, 0.998779f, 0.999023f, 0.999268f, 0.999512f, 0.999756f, 
    }
};

const WaveShaperDataType waveShaperCurvedOverdrive = {
    .transferFunctionPoints = {
        -1.000000f, -0.994792f, -0.989586f, -0.984827f, -0.980801f, -0.977448f, -0.974706f, -0.972515f, -0.970816f, -0.969549f, -0.968652f, -0.968066f, -0.967730f, -0.967585f, -0.967569f, -0.967623f, -0.967687f, -0.967700f, -0.967602f, -0.967332f, -0.966831f, -0.966039f, -0.964894f, -0.963337f, -0.961308f, -0.958745f, -0.955590f, -0.951782f, -0.947260f, -0.941964f, -0.935834f, -0.928810f, -0.920832f, -0.911839f, -0.901771f, -0.890568f, -0.878169f, -0.864515f, -0.849545f, -0.833198f, -0.815415f, -0.796136f, -0.775299f, -0.752846f, -0.728715f, -0.702846f, -0.675179f, -0.645654f, -0.614211f, -0.580790f, -0.545329f, -0.507769f, -0.468750f, -0.429688f, -0.390625f, -0.351562f, -0.312500f, -0.273438f, -0.234375f, -0.195312f, -0.156250f, -0.117188f, -0.078125f, -0.039062f, 0.000000f, 0.039062f, 0.078125f, 0.117188f, 0.156250f, 0.195312f, 0.234375f, 0.273438f, 0.312500f, 0.351562f, 0.390625f, 0.429688f, 0.468750f, 0.507769f, 0.545329f, 0.580790f, 0.614211f, 0.645654f, 0.675179f, 0.702846f, 0.728715f, 0.752846f, 0.775299f, 0.796136f, 0.815415f, 0.833198f, 0.849545f, 0.864515f, 0.878169f, 0.890568f, 0.901771f, 0.911839f, 0.920832f, 0.928810f, 0.935834f, 0.941964f, 0.947260f, 0.951782f, 0.955590f, 0.958745f, 0.961308f, 0.963337f, 0.964894f, 0.966039f, 0.966831f, 0.967332f, 0.967602f, 0.967700f, 0.967687f, 0.967623f, 0.967569f, 0.967585f, 0.967730f, 0.968066f, 0.968652f, 0.969549f, 0.970816f, 0.972515f, 0.974706f, 0.977448f, 0.980801f, 0.984827f, 0.989586f, 0.994792f, 
    }
};

const WaveShaperDataType waveShaperAsymm = {
    .transferFunctionPoints = {
        -0.800000f, -0.799996f, -0.799911f, -0.799653f, -0.799138f, -0.798281f, -0.796998f, -0.795204f, -0.792815f, -0.789746f, -0.785913f, -0.781232f, -0.775617f, -0.768985f, -0.761250f, -0.752330f, -0.742138f, -0.730591f, -0.717604f, -0.703093f, -0.687500f, -0.671875f, -0.656250f, -0.640625f, -0.625000f, -0.609375f, -0.593750f, -0.578125f, -0.562500f, -0.546875f, -0.531250f, -0.515625f, -0.500000f, -0.484375f, -0.468750f, -0.453125f, -0.437500f, -0.421875f, -0.406250f, -0.390625f, -0.375000f, -0.359375f, -0.343750f, -0.328125f, -0.312500f, -0.296875f, -0.281250f, -0.265625f, -0.250000f, -0.234375f, -0.218750f, -0.203125f, -0.187500f, -0.171875f, -0.156250f, -0.140625f, -0.125000f, -0.109375f, -0.093750f, -0.078125f, -0.062500f, -0.046875f, -0.031250f, -0.015625f, 0.000000f, 0.015625f, 0.031250f, 0.046875f, 0.062500f, 0.078125f, 0.093750f, 0.109375f, 0.125000f, 0.140625f, 0.156250f, 0.171875f, 0.187500f, 0.203125f, 0.218750f, 0.234375f, 0.250000f, 0.265625f, 0.279836f, 0.289359f, 0.295081f, 0.298122f, 0.299597f, 0.303711f, 0.304688f, 0.305664f, 0.306641f, 0.307617f, 0.308594f, 0.309570f, 0.310547f, 0.311523f, 0.312500f, 0.313477f, 0.314453f, 0.315430f, 0.316406f, 0.317383f, 0.318359f, 0.319336f, 0.320312f, 0.321289f, 0.322266f, 0.323242f, 0.324219f, 0.325195f, 0.326172f, 0.327148f, 0.328125f, 0.329102f, 0.330078f, 0.331055f, 0.332031f, 0.333008f, 0.333984f, 0.334961f, 0.335938f, 0.336914f, 0.337891f, 0.338867f, 0.339844f, 0.340820f, 0.341797f, 0.342773f, 
    }
};

float waveShaperProcessSample(float sampleIn,WaveShaperDataType*data)
{
    uint32_t indx;
    float v1,v2,rem,factor;
    indx = (uint32_t)(sampleIn*7.62939453125e-06f+ 63.5f); // rescale to 0-127
    v1 = data->transferFunctionPoints[indx];
    v2 = data->transferFunctionPoints[indx+1];
    rem = (sampleIn*7.62939453125e-06f+ 63.5f) - (float)indx;
    factor = v1 + (v2-v1)*rem;
    return sampleIn*factor;
}

#endif
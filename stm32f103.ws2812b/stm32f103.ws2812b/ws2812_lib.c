#include <ws2812_lib.h>

bool ws2812_transmit = false;

static const uint8_t dim_curve[256] = 
{
	0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
    6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
    8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11,
    11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15,
    15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20,
    20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26,
    27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35,
    36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 47,
    48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
    63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 81, 82,
    83, 85, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,	
};

// тут хранятся все 24 кадра костра
static uint32_t frames[NUM_OF_FRAMES][32] = 
{ 
    // 1-й кадр
    {   
        0x00f1ebbb, 0x00f1ebbb, 0x00f1ebbb, 0x00efeba2, 0x00efeb88, 0x00eee46b, 0x00c9af27, 0x00a87c00,
        0x00af6b00, 0x00ee8438, 0x00c42b01, 0x00ca1303, 0x00bd0000, 0x00b50000, 0x009f0000, 0x00920300,
        0x00860000, 0x00850900, 0x00710101, 0x006a0608, 0x00590302, 0x00560806, 0x004a0300, 0x00480701, 
        0x00400604, 0x00390706, 0x002f0705, 0x001d0c04, 0x001d0b06, 0x00000000, 0x00000000, 0x00000000   
    },
    
    // 2-й кадр
    {
        0x00f0cab6, 0x00f0cab6 ,0x00f0cab6, 0x00ecc89c, 0x00ecc888, 0x00f2c670, 0x00f9c052, 0x00ffe24f,   
        0x00c38705, 0x009f4400 ,0x00a72800, 0x00b21d00, 0x00af1200, 0x00a90e00, 0x009d0c00, 0x00950b00,
        0x00930d02, 0x00890c06 ,0x00730301, 0x006b0707, 0x005c0402, 0x00580904, 0x004b0400, 0x00470600,                                         
        0x00400604, 0x00380806 ,0x002e0805, 0x00260905, 0x001f0604, 0x00000000, 0x00000000, 0x00000000 
    },                                    
   
    // 3-й кадр
    {
        0x00f4f3bb, 0x00f4f3bb ,0x00f4f3bb, 0x00f4eca3, 0x00f1dd87, 0x00ffe574, 0x00f1d43a, 0x00d3b20d,   
        0x00cfa616, 0x00d19c1a ,0x00ce8a0d, 0x00da881a, 0x00b75500, 0x00a53e00, 0x00841c00, 0x00801200, 
        0x00870a00, 0x007c0000 ,0x00810a0c, 0x006c0107, 0x00620307, 0x00540000, 0x00500700, 0x00400100,                                          
        0x00350200, 0x00370d0f ,0x0025060b, 0x0021080b, 0x001f0b0a, 0x00000000, 0x00000000, 0x00000000 
    },
    
    // 4-й кадр
    {
        0x00ebeab1, 0x00ebeab1 ,0x00ebeab1, 0x00f3eba2, 0x00f7e58f, 0x00f2d762, 0x00ffe03a, 0x00f0cc14,
        0x00ffdc2e, 0x00ffdb32 ,0x00fcd027, 0x00ffd032, 0x00ffd44d, 0x00ffc859, 0x00f9953d, 0x009c2900, 
        0x008f0f00, 0x00880400 ,0x00780000, 0x00780502, 0x00650000, 0x00630a06, 0x004e0100, 0x004e0e05, 
        0x003d0705, 0x00390d0e ,0x0029060a, 0x0024070b, 0x00220a0a, 0x00000000, 0x00000000, 0x00000000 
    }, 
    
    // 5-й кадр
    {
        0x00ece8ab, 0x00ece8ab ,0x00ece8ab, 0x00f0e69f, 0x00ffee9e, 0x00f3dd6e, 0x00fee445, 0x00f9d71e,
        0x00fed219, 0x00ffd91a ,0x00fad70b, 0x00f0cf06, 0x00ffda23, 0x00e6ae0f, 0x00ffae2c, 0x00ffa13a, 
        0x00ce5d0d, 0x00a63200 ,0x00800b00, 0x00780900, 0x005c0000, 0x005f0700, 0x004c0200, 0x00440500, 
        0x003d0705, 0x00370b0c ,0x002d0609, 0x00260607, 0x00250906, 0x00000000, 0x00000000, 0x00000000 
    },

    // 6-й кадр
    {
        0x00f3ebad, 0x00f3ebad ,0x00f3ebad, 0x00faeba6, 0x00f4e594, 0x00f0df73, 0x00f6e347, 0x00f3d623,
        0x00ffd520, 0x00fbcc0d ,0x00ffe313, 0x00fedf0b, 0x00fddb0f, 0x00f3c70c, 0x00e39f00, 0x00d88200, 
        0x00dd7f0d, 0x00cb6910 ,0x00bd561b, 0x008d2400, 0x006a0a00, 0x00560000, 0x00550c00, 0x004b0d02, 
        0x003d0603, 0x00370707 ,0x00330708, 0x002d0706, 0x00260702, 0x00000000, 0x00000000, 0x00000000 
    },

    // 7-й кадр
    {
        0x00efe2ab, 0x00efe2ab ,0x00efe2ab, 0x00f2e19b, 0x00fff39d, 0x00e8de6d, 0x00e9e047, 0x00f4dd35,
        0x00e6b918, 0x00e4aa08 ,0x00e3ac00, 0x00eab900, 0x00ffe723, 0x00fad20c, 0x00ffd117, 0x00ffc118, 
        0x00eba512, 0x00d68913 ,0x00f09a47, 0x00d97e47, 0x00ad502e, 0x00630c00, 0x00520600, 0x00450200, 
        0x00410602, 0x00390303 ,0x00370506, 0x00310502, 0x00280600, 0x00000000, 0x00000000, 0x00000000
    },  

    // 8-й кадр
    {
        0x00f7f9ba, 0x00f7f9ba ,0x00f7f9ba, 0x00e9ee9e, 0x00eded93, 0x00f3e97a, 0x00ffe65a, 0x00b58f00,
        0x00a8840c, 0x00bd961d ,0x00e2b824, 0x00d2a702, 0x00d3a900, 0x00ffda35, 0x00f3c532, 0x00ffcf4a, 
        0x00ffbf41, 0x00ffbb4f ,0x00c2762a, 0x006a1800, 0x00771400, 0x00851f1b, 0x005b0101, 0x004e0307, 
        0x003d0508, 0x00360509 ,0x003e090f, 0x00340306, 0x00280702, 0x00000000, 0x00000000, 0x00000000 
    }, 

    //9-й кадр
    {
        0x00e9ebac, 0x00e9ebac ,0x00e9ebac, 0x00eaf098, 0x00e9ec81, 0x00efe665, 0x00ffe64d, 0x00d2aa0d,
        0x00b38a00, 0x008f6300 ,0x007f4e00, 0x00a77000, 0x00c89200, 0x00d59c07, 0x00dd9d15, 0x00f9ab2d, 
        0x00f89722, 0x00e37514 ,0x00a53400, 0x008a1400, 0x00800500, 0x00720000, 0x00660000, 0x00570000, 
        0x00450000, 0x00400003 ,0x003e0005, 0x00360204, 0x00290600, 0x00000000, 0x00000000, 0x00000000 
    }, 

    // 10-й кадр
    {
        0x00edefad, 0x00edefad ,0x00edefad, 0x00eef598, 0x00ecf17b, 0x00e9e256, 0x00ffe747, 0x00ffd62e,
        0x00ffd32f, 0x00f3c529 ,0x00f3ba2d, 0x00e7a721, 0x00fbb931, 0x00fbb22f, 0x00eb981e, 0x00e38313, 
        0x00e5710a, 0x00e2610e ,0x00e25923, 0x00d64a29, 0x00c43726, 0x009e1910, 0x00830e04, 0x006e0700, 
        0x005c0107, 0x00510000 ,0x00470000, 0x003d0101, 0x00300904, 0x00000000, 0x00000000, 0x00000000
    }, 

    // 11-й кадр
    {
        0x00f1f3aa, 0x00f1f3aa ,0x00f1f3aa, 0x00eeef92, 0x00eaea7a, 0x00ece45f, 0x00eed53b, 0x00fad637,
        0x00f7d23b, 0x00ffd143 ,0x00ffcd4b, 0x00ffbd3e, 0x00efa925, 0x00d88b15, 0x00f79c49, 0x00bc530f, 
        0x00e67227, 0x00e06529 ,0x00d55e40, 0x00d05546, 0x00cd4a36, 0x00bf3c28, 0x00bb402e, 0x00b24234, 
        0x0088251f, 0x008b332f ,0x0076281e, 0x005a1c11, 0x00380b08, 0x00000000, 0x00000000, 0x00000000 
    },

    // 12-й кадр
    {
        0x00f0f0a6, 0x00f0f0a6, 0x00f0f0a6, 0x00e6e684, 0x00eef170, 0x00f1ec54, 0x00fce43a, 0x00fbd727,
        0x00efc91c, 0x00fccc2c ,0x00f7b72f, 0x00ce8407, 0x00ba6c00, 0x00b55f00, 0x00cb6415, 0x00af3b00, 
        0x00a52a00, 0x00bf3e14 ,0x00c43c2c, 0x00d54c44, 0x00d34e3f, 0x00c1422f, 0x00ad3622, 0x00942615, 
        0x00821b12, 0x00731611 ,0x00520300, 0x003e0100, 0x00340810, 0x00000000, 0x00000000, 0x00000000 
    },

    // 13-й кадр
    {
        0x00f7f0a8, 0x00f7f0a8 ,0x00f7f0a8, 0x00d6d46f, 0x00cfd245, 0x00dcd731, 0x00f6e02b, 0x00ffdf25,
        0x00ffde25, 0x00eec015 ,0x00e4a617, 0x00eea525, 0x00efa027, 0x00ee922b, 0x00e3752a, 0x00c04712, 
        0x00a72d04, 0x00830500 ,0x00940b05, 0x00860000, 0x00770000, 0x00780900, 0x00720e00, 0x005d0000, 
        0x005a0000, 0x00530103 ,0x00420002, 0x00340106, 0x0029040c, 0x00000000, 0x00000000, 0x00000000 
    },

    // 14-й кадр
    {
        0x00f9f1a9, 0x00f9f1a9 ,0x00f9f1a9, 0x00e7e27c, 0x00cfd140, 0x00dfcf22, 0x00d9c30b, 0x00e5c407,
        0x00e1bb00, 0x00e4b608 ,0x00e4a613, 0x00eca624, 0x00e7981f, 0x00e98d28, 0x00f07e37, 0x00f87b4b, 
        0x00eb7253, 0x00c94e3e ,0x00860000, 0x008b090b, 0x006c0000, 0x005f0000, 0x00630c03, 0x00580801, 
        0x004c0000, 0x00480105 ,0x003c070d, 0x00300610, 0x0024030e, 0x00000000, 0x00000000, 0x00000000 
    },

    // 15-й кадр
    {
        0x00fceaa8, 0x00fceaa8 ,0x00fceaa8, 0x00f0e089, 0x00fdf179, 0x00e4d641, 0x00e0cc1f, 0x00e1ca1a,
        0x00cfb61e, 0x00e2bb30 ,0x00dfa118, 0x00c07300, 0x00d17b0a, 0x00d47b21, 0x00b45a1e, 0x00a84621, 
        0x00ad3f26, 0x00c24d3c ,0x00cf564e, 0x00730100, 0x00650604, 0x00540000, 0x00550504, 0x00510b09, 
        0x00420807, 0x00350709 ,0x00260309, 0x0022030b, 0x002c040f, 0x00000000, 0x00000000, 0x00000000 
    },

    // 16-й кадр
    {
        0x00fdeeab, 0x00fdeeab ,0x00fdeeab, 0x00f9e991, 0x00f3e568, 0x00f6e74c, 0x00f6e034, 0x00fadf2e,
        0x00fadd37, 0x00dab316 ,0x00dca210, 0x00f6ad2a, 0x00f4a32e, 0x00a14800, 0x00802200, 0x00892600, 
        0x008c2006, 0x007c0800 ,0x00810600, 0x00780502, 0x005c0100, 0x00631513, 0x004a0000, 0x00500a0a, 
        0x003f0102, 0x003d0d0d ,0x002a0a0d, 0x0027080d, 0x002e070c, 0x00000000, 0x00000000, 0x00000000 
    },

    // 17-й кадр
    {
        0x00f8f0a8, 0x00f8f0a8 ,0x00f8f0a8, 0x00f4eb90, 0x00fbf178, 0x00fff261, 0x00fbe43e, 0x00f7d929,
        0x00f9da2a, 0x00fbd52c ,0x00fccc2c, 0x00fbc332, 0x00b36f00, 0x00ad5d00, 0x00923700, 0x00882300, 
        0x007f1500, 0x00831508 ,0x00710400, 0x00700805, 0x00600504, 0x00560202, 0x005c0d10, 0x004e0607, 
        0x00420202, 0x003a0402 ,0x00300703, 0x002b0a05, 0x002a0806, 0x00000000, 0x00000000, 0x00000000 
    },

    // 18-й кадр
    {
        0x00ebe8a1, 0x00ebe8a1 ,0x00ebe8a1, 0x00f1eb95, 0x00b7ad3c, 0x00807000, 0x00ffea4c, 0x00f8da2e,
        0x00f9d829, 0x00ffdb2e ,0x00ebc51c, 0x00cd9f03, 0x00d59b15, 0x00df9929, 0x00ed9744, 0x00ca6a29, 
        0x00bb582e, 0x00801d00 ,0x006f0f00, 0x00690c00, 0x00620a00, 0x005a0703, 0x00570909, 0x00440000, 
        0x00450506, 0x003c0604 ,0x00320701, 0x00280700, 0x00230801, 0x00000000, 0x00000000, 0x00000000 
    },

    // 19-й кадр
    {
        0x00ebe7aa, 0x00ebe7aa ,0x00ebe7aa, 0x00f0ea9c, 0x00bbb24d, 0x00feef72, 0x00edd443, 0x00edd020e,
        0x00dbbb12, 0x00caa800 ,0x00d9b70d, 0x00e8c11e, 0x00fbc934, 0x00fec240, 0x00ffb44c, 0x00ffb860, 
        0x00ffb262, 0x00ffb875 ,0x00d17a44, 0x008a360a, 0x006c1800, 0x006b190b, 0x00500200, 0x00570f12, 
        0x00400205, 0x00390507 ,0x00310909, 0x002a0d09, 0x00210e08, 0x00000000, 0x00000000, 0x00000000 
    },

    // 20-й кадр
    {
        0x00efeab3, 0x00efeab3 ,0x00efeab3, 0x00f8f2a8, 0x00eae17c, 0x00fdf070, 0x00ebd63d, 0x00e5cd25,
        0x00ddc213, 0x00d5b805 ,0x00fee030, 0x00f2cf29, 0x00ffd43d, 0x00ffc643, 0x00ffba51, 0x00e9983d, 
        0x00d88330, 0x00cb752c ,0x00d2793d, 0x00d77c4f, 0x00b35839, 0x00680f00, 0x005f0a05, 0x00500001, 
        0x004c0608, 0x00410608 ,0x00330706, 0x00270805, 0x001e0b05, 0x00000000, 0x00000000, 0x00000000 
    },

    // 21-й кадр
    {
        0x00efe9b7, 0x00efe9b7 ,0x00efe9b7, 0x00f4eea2, 0x00e7e171, 0x00efe456, 0x00e4d427, 0x00e1ce10,
        0x00e2cb09, 0x00fde425 ,0x00eed41d, 0x00f8d732, 0x00ffd74a, 0x00be8512, 0x00ac620d, 0x00b15c1b, 
        0x00c16228, 0x00c4602f ,0x009e340d, 0x0092260a, 0x00760800, 0x006c0100, 0x00670100, 0x005d0000, 
        0x00520000, 0x00470100 ,0x003a0300, 0x002f0600, 0x00260903, 0x00000000, 0x00000000, 0x00000000 
    },

    // 22-й кадр
    {
        0x00f3eeb7, 0x00f3eeb7 ,0x00f3eeb7, 0x00e8e391, 0x00e0dc61, 0x00d4cb30, 0x00dfd118, 0x00eedd14,
        0x00f1dc13, 0x00f0d814 ,0x00f3d924, 0x00f3d131, 0x00956700, 0x007f4200, 0x008d3f00, 0x00903200, 
        0x00881d00, 0x00861200 ,0x00901700, 0x008a0d00, 0x0097190b, 0x00951a12, 0x00901d16, 0x00821913, 
        0x005f0200, 0x00520300 ,0x00430300, 0x00340200, 0x00290400, 0x00000000, 0x00000000, 0x00000000 
    },

    // 23-й кадр
    {
        0x00efeaa6, 0x00efeaa6 ,0x00efeaa6, 0x00f4f08f, 0x00ece863, 0x00f2ea49, 0x00f5e62b, 0x00efdc14,
        0x00f8e01c, 0x00f0d317 ,0x00ffdf32, 0x00bd9400, 0x00b47e00, 0x00ffbc59, 0x00e8944e, 0x008e2900, 
        0x00971e00, 0x00b6320b ,0x00b34827, 0x00db4e33, 0x00ca3d29, 0x00cb4333, 0x00bd3f30, 0x00b14231, 
        0x009b3a29, 0x00833020 ,0x00651f13, 0x00490e06, 0x00330301, 0x00000000, 0x00000000, 0x00000000 
    },

    // 24-й кадр
    {
        0x00edea9b, 0x00edea9b ,0x00edea9b, 0x00f3f089, 0x00f8f26a, 0x00f7ee4b, 0x00f8e52f, 0x00f8df20,
        0x00f5d61f, 0x00fad723 ,0x00ffd630, 0x00ffd13b, 0x00ffcd4b, 0x00de9429, 0x00913600, 0x00ae4100, 
        0x00da5c1f, 0x00f56934 ,0x00e85529, 0x00e24c29, 0x00dc482e, 0x00b52a15, 0x00981b09, 0x007c0e00, 
        0x006a0d00, 0x00dc482e ,0x00b52a15, 0x00981b09, 0x007c0e00, 0x00000000, 0x00000000, 0x00000000 
    }

    //      1           2           3           4           5           6           7           8      
    //      9           10          11          12          13          14          15          16  
    //      17          18          19          20          21          22          23          24  
    //      25          26          27          28          29          30          31          32 
}; 

/* simple delay counter to waste time, don't rely on for accurate timing */
void Delay(__IO uint32_t nCount)
{
	while (nCount--) {
	}
}

/* Transmit the frambuffer with buffersize number of bytes to the LEDs 
 * buffersize = (#LEDs / 16) * 24 */
void WS2812_sendbuf(uint32_t buffersize)
{		
	// transmission complete flag, indicate that transmission is taking place
	ws2812_transmit = true;
	
	// clear all relevant DMA flags
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2 | DMA1_FLAG_TE2);
	DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_HT5 | DMA1_FLAG_GL5 | DMA1_FLAG_TE5);
	DMA_ClearFlag(DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7);
	
	// configure the number of bytes to be transferred by the DMA controller
	DMA_SetCurrDataCounter(DMA1_Channel2, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel5, buffersize);
	DMA_SetCurrDataCounter(DMA1_Channel7, buffersize);
	
	// clear all TIM2 flags
	TIM2->SR = 0;
	
	// enable the corresponding DMA channels
	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);
	DMA_Cmd(DMA1_Channel7, ENABLE);
	
	// IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
	TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
	TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);
	
	// preload counter with 29 so TIM2 generates UEV directly to start DMA transfer
	TIM_SetCounter(TIM2, 29);
	
	// start TIM2
	TIM_Cmd(TIM2, ENABLE);
}

/* This function sets the color of a single pixel in the framebuffer 
 * 
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display 
 */
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint32_t rgb)
{
	uint8_t i;
	uint8_t red   = (uint8_t)((rgb & 0x00FF0000) >> 16);
	uint8_t green = (uint8_t)((rgb & 0x0000FF00) >> 8);
	uint8_t blue  = (uint8_t)(rgb & 0x000000FF);

	for (i = 0; i < 8; i++)
	{
		// clear the data for pixel 
		WS2812_IO_framedata[((column * 24) + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] &= ~(0x01 << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] &= ~(0x01 << row);
		// write new data for pixel
		WS2812_IO_framedata[((column * 24) + i)] |= ((((green << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 8 + i)] |= ((((red << i) & 0x80) >> 7) << row);
		WS2812_IO_framedata[((column * 24) + 16 + i)] |= ((((blue << i) & 0x80) >> 7) << row);
	}
}

/* This function is a wrapper function to set all LEDs in the complete row to the specified color
 * 
 * Arguments:
 * row = the channel number/LED strip to set the color of from 0 to 15
 * columns = the number of LEDs in the strip to set to the color from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setRow(uint8_t row, uint16_t columns, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < columns; i++)
	{
		WS2812_framedata_setPixel(row, i, rgb);
	}
}

/* This function is a wrapper function to set all the LEDs in the column to the specified color
 * 
 * Arguments:
 * rows = the number of channels/LED strips to set the row in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display 
 */
void WS2812_framedata_setColumn(uint8_t rows, uint16_t column, uint32_t rgb)
{
	uint8_t i;
	for (i = 0; i < rows; i++)
	{
		WS2812_framedata_setPixel(i, column, rgb);
	}
}

// Извлекает  нужный цвет в формате uint8_t из 32 битной переменной содержащей три цвета 0x00RRGGBB
uint8_t put_rgb_mask(uint32_t input_tone, rgb_mask color)
{	
	uint8_t output = 0x00;
	uint32_t input = input_tone; 
	
	switch (color)
	{
	case RED :
		output = (uint8_t)((input & 0x00FF0000) >> 16);
		break;
	case GREEN :
		output = (uint8_t)((input & 0x0000FF00) >> 8);
		break;
	case BLUE :
		output = (uint8_t)(input & 0x000000FF);
		break;
	default:
		break;
	}

	return output;
}

// извлекает тип операции с цветом: вычесть его или сложить. 
// Хранится в самой старшей части старшего байта корректирующей цвет посылки.
rgb_operation eject_operation(uint32_t input_tone, rgb_mask color)
{
	rgb_operation result = 0x00;
	uint32_t input = input_tone;
	
	switch (color)
	{
	case RED :
		if ((input & 0x40000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		
		break;
		
	case GREEN :
		if ((input & 0x20000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		break;
		
	case BLUE :
		
		if ((input & 0x10000000) != 0)
		{
			result = ADD;	
		}
		else
		{
			result = SUB;
		}
		break;
		
	default:
		break;
	}
	
	return result;
}

// первый костер был записан в огроммный массив
// но если отображать его напрямую, то требуется коррекция цвета
// иначе будет слишком блеклый, вот для этого и была создана эта функция.
uint32_t tone_correction_func(uint32_t input_tone, 
	uint8_t r_index,
	rgb_operation r_op,
	uint8_t g_index,
	rgb_operation g_op,
	uint8_t b_index,
	rgb_operation b_op)
{
	uint32_t out = 0x00000000;
	
	uint8_t red   = put_rgb_mask(input_tone, RED);
	uint8_t green = put_rgb_mask(input_tone, GREEN);
	uint8_t blue  = put_rgb_mask(input_tone, BLUE);
	
	switch (r_op)
	{
	case ADD:
		
	// проверяем на переполнение.
		if ((red + r_index) <= 0xFF)
		{
			red += r_index;			
		}
		else
		{
			red = 0xFF;
		}

		break;
		
	case SUB:
		
	// проверяем на переполнение.
		if ((red - r_index) >= 0x00)
		{
			red -= r_index;			
		}
		else
		{
			red = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (g_op)
	{
	case ADD:
		
	    // проверяем на переполнение.
		if ((green + g_index) <= 0xFF)
		{
			green += g_index;			
		}
		else
		{
			green = 0xFF;
		}

		break;
		
	case SUB:
		
	    // проверяем на переполнение.
		if ((green - g_index) >= 0x00)
		{
			green -= g_index;			
		}
		else
		{
			green = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	switch (b_op)
	{
	case ADD:
		
	    // проверяем на переполнение.
		if ((blue + b_index) <= 0xFF)
		{
			blue += b_index;			
		}
		else
		{
			blue = 0xFF;
		}

		break;
		
	case SUB:
		
	    // проверяем на переполнение.
		if ((blue - b_index) >= 0x00)
		{
			blue -= b_index;			
		}
		else
		{
			blue = 0x00;
		}
		
		break;		
	
	default:
		break;
	}
	
	out |= ((uint32_t)red << 16);
	out |= ((uint32_t)green << 8);
	out |=  (uint32_t)blue;
	
	return out;
}

/*------------------------------------------------------------------------------
   Конвертируем RGB буффер в DMA буффер
 ------------------------------------------------------------------------------*/
void convert_rgb_to_dma_buf(struct CRGB * buf)
{
	static uint8_t i;
	
	for (i = 0; i < NUMOFLEDS; i++)
	{		
		// раскомментить в зависимости от используемого канала.
	  //WS2812_framedata_setPixel(0, i, buf[i]);
	  //WS2812_framedata_setPixel(1, i, buf[i]);
	  //WS2812_framedata_setPixel(2, i, buf[i]);
	  //WS2812_framedata_setPixel(3, i, buf[i]);
		WS2812_framedata_setPixel(4, i, buf[i].rgb);
		WS2812_framedata_setPixel(5, i, buf[i].rgb);
		WS2812_framedata_setPixel(6, i, buf[i].rgb);
		WS2812_framedata_setPixel(7, i, buf[i].rgb);
	}
	
	WS2812_sendbuf(BUFFERSIZE);	
	
	// ждем пока окончится передача в DMA одного кадра.
	Delay(1500);
	
	// передача окончена
	ws2812_transmit = false;
}


/*
struct CRGB hsv_to_rgb(struct CHSV hsv) 
{
	uint8_t    r;
	uint8_t    g;
	uint8_t    b;
	uint8_t    base;
	struct CRGB rgb;

	hsv.val = dim_curve[hsv.val];
	hsv.sat = 255 - dim_curve[255 - hsv.sat];


	if (hsv.sat == 0) // Acromatic color (gray). Hue doesn't mind.
	{
		rgb = hsv.val | (hsv.val << 8) | (hsv.val << 16);
	}
	else
	{
		base = ((255 - hsv.sat) * hsv.val) >> 8;
		switch (hsv.hue / 60)
		{
		case 0:
			r = hsv.val;
			g = (((hsv.val - base) * hsv.hue) / 60) + base;
			b = base;
			break;
		case 1:
			r = (((val - base) * (60 - (hue % 60))) / 60) + base;
			g = val;
			b = base;
			break;
		case 2:
			r = base;
			g = val;
			b = (((val - base) * (hue % 60)) / 60) + base;
			break;
		case 3:
			r = base;
			g = (((val - base) * (60 - (hue % 60))) / 60) + base;
			b = val;
			break;
		case 4:
			r = (((val - base) * (hue % 60)) / 60) + base;
			g = base;
			b = val;
			break;
		case 5:
			r = val;
			g = base;
			b = (((val - base) * (60 - (hue % 60))) / 60) + base;
			break;
		}
		rgb = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
	}
	return rgb;
}
*/



/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB в целочисленной арифмерите
 
  hue        : 0..191(0xBF)	// цветовой тон, (например, красный, зелёный или сине-голубой). Варьируется в пределах 0—191
  saturation : 0..255	// Насыщенность. Чем больше этот параметр, тем «чище» цвет, поэтому этот параметр иногда называют чистотой цвета.
  value      : 0..255   // или Brightness — яркость. Также задаётся в пределах 0—255
 ------------------------------------------------------------------------------*/

#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)

void hsv2rgb (struct CHSV * hsv, struct CRGB * rgb)
{
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
	uint8_t value = dim_curve[ hsv->val ];
	uint8_t saturation = hsv->sat;

	// The brightness floor is minimum number that all of
	// R, G, and B will be set to.
	uint8_t invsat = dim_curve[ 255 - saturation];
	uint8_t brightness_floor = (value * invsat) / 256;

	// The color amplitude is the maximum amount of R, G, and B
	// that will be added on top of the brightness_floor to
	// create the specific hue desired.
	uint8_t color_amplitude = value - brightness_floor;

	// Figure out which section of the hue wheel we're in,
	// and how far offset we are withing that section
	uint8_t section = hsv->hue / HSV_SECTION_3; // 0..2
	uint8_t offset = hsv->hue % HSV_SECTION_3;  // 0..63

	uint8_t rampup = offset; // 0..63
	uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

	// We now scale rampup and rampdown to a 0-255 range -- at least
	// in theory, but here's where architecture-specific decsions
	// come in to play:
	// To scale them up to 0-255, we'd want to multiply by 4.
	// But in the very next step, we multiply the ramps by other
	// values and then divide the resulting product by 256.
	// So which is faster?
	//   ((ramp * 4) * othervalue) / 256
	// or
	//   ((ramp    ) * othervalue) /  64
	// It depends on your processor architecture.
	// On 8-bit AVR, the "/ 256" is just a one-cycle register move,
	// but the "/ 64" might be a multicycle shift process. So on AVR
	// it's faster do multiply the ramp values by four, and then
	// divide by 256.
	// On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
	// faster to NOT multiply the ramp values by four, and just to
	// divide the resulting product by 64 (instead of 256).
	// Moral of the story: trust your profiler, not your insticts.

	// Since there's an AVR assembly version elsewhere, we'll
	// assume what we're on an architecture where any number of
	// bit shifts has roughly the same cost, and we'll remove the
	// redundant math at the source level:

	//  // scale up to 255 range
	//  //rampup *= 4; // 0..252
	//  //rampdown *= 4; // 0..252

	// compute color-amplitude-scaled-down versions of rampup and rampdown
	uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
	uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

	// add brightness_floor offset to everything
	uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
	uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


	if( section )
	{
	    if( section == 1)
	    {
	        // section 1: 0x40..0x7F
	        rgb->r = brightness_floor;
	        rgb->g = rampdown_adj_with_floor;
	        rgb->b = rampup_adj_with_floor;
        } 
	    else
	    {
            // section 2; 0x80..0xBF
		    rgb->r = rampup_adj_with_floor;
		    rgb->g = brightness_floor;
		    rgb->b = rampdown_adj_with_floor;
        }
    } 
	else 
	{
        // section 0: 0x00..0x3F
		rgb->r = rampdown_adj_with_floor;
		rgb->g = rampup_adj_with_floor;
		rgb->b = brightness_floor;
    }
}

/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB в целочисленной арифмерите
 
  hue        : 0..360	// цветовой тон, (например, красный, зелёный или сине-голубой). Варьируется в пределах 0—360
  saturation : 0..255	// Насыщенность. Чем больше этот параметр, тем «чище» цвет, поэтому этот параметр иногда называют чистотой цвета.
  value      : 0..255   // или Brightness — яркость. Также задаётся в пределах 0—255
 ------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  Перемещение светодиодов в заданную сторону с заданной задержкой и заданное количество шагов
------------------------------------------------------------------------------*/
void move_leds(uint32_t n, uint32_t dir, uint32_t del, struct CRGB  * buf)
{
	uint32_t i, k;
	struct CRGB tmpLED;

	for (k = 0; k < n; k++)
	{
		convert_rgb_to_dma_buf(buf);

		if (dir == 0)
		{
		  // Перемещаем огни
			tmpLED = buf[0];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != (NUMOFLEDS - 1))
				{
					buf[i] = buf[i + 1];
				}
				else
				{
					buf[i] = tmpLED;
				}
			}
		}
		else
		{
		  // Перемещаем огни
			tmpLED = buf[NUMOFLEDS - 1];
			for (i = 0; i < NUMOFLEDS; i++)
			{
				if (i != 0)
				{
					buf[NUMOFLEDS - i] = buf[NUMOFLEDS - i - 1];
				}
				else
				{
					buf[0] = tmpLED;
				}
			}
		}
		
		Delay(del);
	}
}

/*------------------------------------------------------------------------------
  Заполняем радугой 
 ------------------------------------------------------------------------------*/
void fill_rainbow(const struct CHSV * hsv, struct CRGB * rgb)
{
	uint32_t i;
	struct CHSV hsv_buf = * hsv;

	// Записываем в буфер начальную последовательность
	for (i = 0; i < NUMOFLEDS; i++)
	{
		hsv_buf.hue = ((0xBF * i) / NUMOFLEDS);

		hsv2rgb(&hsv_buf, &rgb[i]);			
	}
}

/*------------------------------------------------------------------------------
  Бегущая радуга 
 ------------------------------------------------------------------------------*/
void running_rainbow(struct CRGB  * buf)
{
	struct CHSV hsv_buf = { 0xff, 0xff, 0xff };
	
	while (1)
	{
		fill_rainbow( &hsv_buf, buf);
		move_leds(NUMOFLEDS, 1, 200000, buf);
		fill_rainbow( &hsv_buf, buf);
		move_leds(NUMOFLEDS, 1, 200000, buf);		
	}	
}


/*------------------------------------------------------------------------------
  вращающаяся радуга 
 ------------------------------------------------------------------------------*/
void rotating_rainbow(struct CRGB  * rgb)
{
	uint8_t i;	
	struct CHSV hsv_buf = { 0xff, 0xff, 0 };
	
	while (1)
	{		
		for (i = 0; i < NUMOFLEDS; i++)
		{		
			hsv2rgb(&hsv_buf, &rgb[i]);	
		}
		
		convert_rgb_to_dma_buf(rgb);
		
		Delay(40000);
				
		if (0xBF <= (++hsv_buf.hue))
		{
			hsv_buf.hue = 0;
		}
	}	
}

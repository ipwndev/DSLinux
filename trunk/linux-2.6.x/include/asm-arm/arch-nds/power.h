
#define POWER_CR       (*(volatile u16*)0x04000304)

#define POWER_LCD	 (1<<0)
#define POWER_2D         (1<<1)
#define POWER_MATRIX     (1<<2)
#define POWER_3D_CORE    (1<<3)
#define POWER_2D_SUB     (1<<9)
#define POWER_SWAP_LCDS  (1<<15)


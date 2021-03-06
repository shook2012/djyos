//----------------------------------------------------
// Copyright (c) 2014, SHENZHEN PENGRUI SOFT CO LTD. All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
// Copyright (c) 2014 著作权由深圳鹏瑞软件有限公司所有。著作权人保留一切权利。
//
// 这份授权条款，在使用者符合下列条件的情形下，授予使用者使用及再散播本
// 软件包装原始码及二进位可执行形式的权利，无论此包装是否经改作皆然：
//
// 1. 对于本软件源代码的再散播，必须保留上述的版权宣告、本条件列表，以
//    及下述的免责声明。
// 2. 对于本套件二进位可执行形式的再散播，必须连带以文件以及／或者其他附
//    于散播包装中的媒介方式，重制上述之版权宣告、本条件列表，以及下述
//    的免责声明。

// 免责声明：本软件是本软件版权持有人以及贡献者以现状（"as is"）提供，
// 本软件包装不负任何明示或默示之担保责任，包括但不限于就适售性以及特定目
// 的的适用性为默示性担保。版权持有人及本软件之贡献者，无论任何条件、
// 无论成因或任何责任主义、无论此责任为因合约关系、无过失责任主义或因非违
// 约之侵权（包括过失或其他原因等）而起，对于任何因使用本软件包装所产生的
// 任何直接性、间接性、偶发性、特殊性、惩罚性或任何结果的损害（包括但不限
// 于替代商品或劳务之购用、使用损失、资料损失、利益损失、业务中断等等），
// 不负任何责任，即在该种使用已获事前告知可能会造成此类损害的情形下亦然。
//-----------------------------------------------------------------------------
//所属模块:  CPU初始化
//作者：     lst
//版本：      V1.0.0
//初始版本完成日期：2013-05-27
//文件描述:  cpu时钟初始化
//其他说明:  无
//修订历史:
//2. ...
//1. 日期: 2014-01-20
//   作者: lst
//   新版本号：V1.0.0
//   修改说明: 本版是初版,用于验证目的
//		下一版需增加判断时钟是否已经起振的代码
//------------------------------------------------------
#include "stdint.h"
#include "cpu_peri.h"
#include "hard-exp.h"
#include "arch_feature.h"
#include "sam4e.h"
#include "core_cmFunc.h"

#ifndef __CHECK_DEVICE_DEFINES
#define __CHECK_DEVICE_DEFINES
#endif

#include "core_cm4.h"

extern   uint32_t   msp_top[ ];
extern void __set_PSP(uint32_t topOfProcStack);
extern void __set_PRIMASK(uint32_t priMask);
extern void __set_FAULTMASK(uint32_t faultMask);
extern void __set_CONTROL(uint32_t control);

extern void Load_Preload(void);
struct ScbReg volatile * const startup_scb_reg
                        = (struct ScbReg *)0xe000ed00;
void Startup_NMI(void)
{
    while(1);
}
void Startup_Hardfault(void)
{
    while(1);
}
void Init_Cpu(void);
//为什么是256-3？ 编译生成rodata时，会在gc_u32StartupExpTable后面增加12字节的rodata
//不知道为什么？如果
const u32 gc_u32StartupExpTable[256] =
{
    (u32)msp_top,
    (u32)Init_Cpu,
    (u32) Startup_NMI,
    (u32) Startup_Hardfault
};

void Init_Cpu(void)
{
	__set_PSP((uint32_t)msp_top);
	__set_PRIMASK(1);
	__set_FAULTMASK(1);
	__set_CONTROL(0);

    #if (CN_CPU_OPTIONAL_FPU == 1)
	startup_scb_reg->CPACR = (3UL << 20)|(3UL << 22);    //使能FPU
	startup_scb_reg->FPCCR = (1UL << 31);                //关闭lazy stacking
    #endif
    switch(startup_scb_reg->CPUID)
    {
//        case cn_revision_r0p1://todo
//            break;    //好像没什么要做的
    }

    //关狗
    WDT->WDT_MR = WDT_MR_WDDIS;

    void System_ClkInit(void);
    System_ClkInit();
    Load_Preload();
}


#define EEFC_FMR_Val    0x00000500      // 0x00000000

#define PMC_SETUP       1               // Reset values:
#define PMC_SCER_Val    0x00000000      // WO register (0x00000001)
#define PMC_PCER0_Val   0x00000000      // WO register (0x00000000)
#define PMC_PCER1_Val   0x00000000      // WO register (0x00000000)
#define CKGR_MOR_Val    0x01003F29      //

// PLL A Freq = (Main CLOCK Freq / DIVA) * (MULA + 1)
// 如下： MULA = 0x1F, DIVA = 0x04, PLLA = 12M / 0x04 * (0x27+1) = 120M
#define CKGR_PLLAR_Val  0x00270604      // 0x00003F00
#define FILL_0          0x00000000      // 0x00000000
#define PMC_MCKR_Val    0x00000002      // 选择 Master Clock Source 为PLLA_CLK
#define PMC_USB_Val     0x00000100      // 0x00000000
#define PMC_PCK0_Val    0x00000001      // 0x00000000
#define PMC_PCK1_Val    0x00000004      // 0x00000000
#define PMC_PCK2_Val    0x00000000      // 0x00000000

void System_ClkInit(void)
{
	EFC->EEFC_FMR = EEFC_FMR_Val;

	PMC->PMC_WPMR = 0x504D4300;                 /* Disable write protect             */

	/* before we change the clocksetup we switch Master Clock Source
     to MAIN_CLK and set prescaler to one
	 */
	PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));     /* Wait for MCKRDY                   */

	PMC->PMC_MCKR = (PMC->PMC_MCKR & ~ PMC_MCKR_PRES_Msk) | PMC_MCKR_PRES_CLK_1;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));     /* Wait for MCKRDY                   */

#if (CKGR_MOR_Val & (CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN))    /* If MOSCRCEN/MOSCXTEN set  */
	PMC->CKGR_MOR  = ((PMC->CKGR_MOR &  CKGR_MOR_MOSCSEL)    |    /* Keep the current MOSCSEL  */
                    (CKGR_MOR_Val  & ~CKGR_MOR_MOSCSEL)    |    /* Set value except MOSCSEL  */
                    (CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN)|    /* and enable bothe OSCs     */
                    (CKGR_MOR_KEY(0x37))                      );
#if (CKGR_MOR_Val & CKGR_MOR_MOSCRCEN)
	while (!(PMC->PMC_SR & PMC_SR_MOSCRCS));   /* Wait for MOSCRCS                   */
#endif
#if (CKGR_MOR_Val & CKGR_MOR_MOSCXTEN)
	while (!(PMC->PMC_SR & PMC_SR_MOSCXTS));   /* Wait for MOSCXTS                   */
#endif
	PMC->CKGR_MOR  =  ((CKGR_MOR_Val      ) |  /* set the desired selection          */
                     (CKGR_MOR_KEY(0x37))  );
	while (!(PMC->PMC_SR & PMC_SR_MOSCSELS));  /* Wait for MOSCSELS                  */
#endif

  /* write PLLBDIV2, PLLADIV2 */
	PMC->PMC_MCKR = ((PMC->PMC_MCKR & ~(PMC_MCKR_PLLADIV2)) |
                   (PMC_MCKR_Val  &  (PMC_MCKR_PLLADIV2))  );
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));    /* Wait for MCKRDY                    */

#if (CKGR_PLLAR_Val & CKGR_PLLAR_MULA_Msk)   /* If PLL is activated                */
	PMC->CKGR_PLLAR = (CKGR_PLLAR_Val | CKGR_PLLAR_ONE);
	while (!(PMC->PMC_SR & PMC_SR_LOCKA));     /* Wait for LOCKA                     */
#endif

	if ((PMC_MCKR_Val & PMC_MCKR_CSS_Msk) >= 2)
	{
		/* Write PRES field only                                                       */
		PMC->PMC_MCKR = ((PMC->PMC_MCKR & ~PMC_MCKR_PRES_Msk) |
                     (PMC_MCKR_Val  &  PMC_MCKR_PRES_Msk)  );
		while (!(PMC->PMC_SR & PMC_SR_MCKRDY));    /* Wait for MCKRDY                  */

		/* Write CSS field only                                                        */
		PMC->PMC_MCKR = ((PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) |
                     (PMC_MCKR_Val  &  PMC_MCKR_CSS_Msk)  );
		while (!(PMC->PMC_SR & PMC_SR_MCKRDY));    /* Wait for MCKRDY                  */
	}
	else
	{
		/* Write CSS field only                                                        */
		PMC->PMC_MCKR = ((PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) |
                     (PMC_MCKR_Val  &  PMC_MCKR_CSS_Msk)  );
		while (!(PMC->PMC_SR & PMC_SR_MCKRDY));    /* Wait for MCKRDY                  */

		/* Write PRES field only                                                       */
		PMC->PMC_MCKR = ((PMC->PMC_MCKR & ~PMC_MCKR_PRES_Msk) |
                     (PMC_MCKR_Val  &  PMC_MCKR_PRES_Msk)  );
		while (!(PMC->PMC_SR & PMC_SR_MCKRDY));    /* Wait for MCKRDY                  */
	}

	PMC->PMC_SCER  = PMC_SCER_Val;
	PMC->PMC_PCER0 = PMC_PCER0_Val;
	PMC->PMC_PCER1 = PMC_PCER1_Val;

#if (PMC_SCER_Val & PMC_SCER_PCK0)
	PMC->PMC_PCK[0] = PMC_PCK0_Val;            /* Write PCK0                         */
	while (!(PMC->PMC_SR & PMC_SR_PCKRDY0));   /* Wait for PCKRDY0                   */
#endif
#if (PMC_SCER_Val & PMC_SCER_PCK1)
	PMC->PMC_PCK[1] = PMC_PCK1_Val;            /* Write PCK1                         */
	while (!(PMC->PMC_SR & PMC_SR_PCKRDY1));   /* Wait for PCKRDY1                   */
#endif
#if (PMC_SCER_Val & PMC_SCER_PCK2)
	PMC->PMC_PCK[2] = PMC_PCK2_Val;            /* Write PCK2                         */
	while (!(PMC->PMC_SR & PMC_SR_PCKRDY2));   /* Wait for PCKRDY2                   */
#endif


  PMC->PMC_WPMR = 0x504D4301;                /* Enable write protect               */
}



// =============================================================================
extern char g_cIbootFlag[];
const char bootflag[]="RunIboot";//要弄成const，若是局部变量，编译器将其放在ROM
void IAP_GpioPinInit(void)
{
}
bool_t IAP_IsForceIboot(void)
{
	return false;
}
bool_t IAP_IsRamIbootFlag(void)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		if(g_cIbootFlag[i]!=bootflag[i])
		{
			return false;
		}
	}
	return true;
}

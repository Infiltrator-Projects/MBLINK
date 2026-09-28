// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_egs53_lookup.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"check failed: %s at %s:%d\n",#x,__FILE__,__LINE__);return 1;}}while(0)
static void setp(uint8_t p[8],unsigned int off,unsigned int len,uint64_t v){unsigned int b;for(b=0U;b<len;++b){unsigned int sb=off+b,from=len-1U-b;uint8_t one=(uint8_t)((v>>from)&UINT64_C(1)),m=(uint8_t)(UINT8_C(1)<<(7U-(sb%8U)));if(one)p[sb/8U]|=m;else p[sb/8U]&=(uint8_t)~m;}}
int main(void){uint8_t b[8]={0};const MblinkMercedesEgs53FrameDefinition *fr;const MblinkMercedesEgs53SignalDefinition *s;MblinkMercedesEgs53DecodedSignal d;
 CHECK(mblink_mercedes_egs53_frame_count()==95U);CHECK(mblink_mercedes_egs53_signal_count()==602U);CHECK(strcmp(mblink_mercedes_egs53_source_revision(),"1b96089660e97c91811b3d9cda6ca6f82b458c69")==0);
 fr=mblink_mercedes_egs53_frame_match_at(UINT32_C(0x2f1),0U);CHECK(fr!=NULL&&strcmp(fr->ecu,"TCM")==0);s=mblink_mercedes_egs53_signal_find(fr,"TxOilTemp");setp(b,s->bit_offset,s->bit_length,UINT64_C(88));CHECK(mblink_mercedes_egs53_decode_signal(s,b,8U,&d));CHECK(d.physical_available&&d.physical_value==38.0);
 memset(b,0,sizeof(b));s=mblink_mercedes_egs53_signal_find(fr,"TSL_Posn_TCM");setp(b,s->bit_offset,s->bit_length,UINT64_C(4));CHECK(mblink_mercedes_egs53_decode_signal(s,b,8U,&d));CHECK(d.enum_available&&strcmp(d.enum_name,"D")==0);
 memset(b,0,sizeof(b));s=mblink_mercedes_egs53_signal_find(fr,"VehDrvProg_TCM_V2");setp(b,s->bit_offset,s->bit_length,UINT64_C(1));CHECK(mblink_mercedes_egs53_decode_signal(s,b,8U,&d));CHECK(d.enum_available&&strcmp(d.enum_name,"COMFORT")==0);
 memset(b,0,sizeof(b));fr=mblink_mercedes_egs53_frame_match_at(UINT32_C(0xf1),0U);s=mblink_mercedes_egs53_signal_find(fr,"EngTrq_Rq_TCM");setp(b,s->bit_offset,s->bit_length,UINT64_C(2400));CHECK(mblink_mercedes_egs53_decode_signal(s,b,8U,&d));CHECK(d.physical_available&&d.physical_value==100.0);
 puts("Mercedes EGS53 isolated lookup tests passed");return 0;}

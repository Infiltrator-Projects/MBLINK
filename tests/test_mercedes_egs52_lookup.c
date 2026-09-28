// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_egs52_lookup.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"check failed: %s at %s:%d\n",#x,__FILE__,__LINE__);return 1;}}while(0)
static void setp(uint8_t p[8],unsigned int off,unsigned int len,uint64_t v){unsigned int b;for(b=0U;b<len;++b){unsigned int sb=off+b,from=len-1U-b;uint8_t one=(uint8_t)((v>>from)&UINT64_C(1)),m=(uint8_t)(UINT8_C(1)<<(7U-(sb%8U)));if(one)p[sb/8U]|=m;else p[sb/8U]&=(uint8_t)~m;}}
int main(void){uint8_t b[8]={0};const MblinkMercedesEgs52FrameDefinition *fr;const MblinkMercedesEgs52SignalDefinition *s;MblinkMercedesEgs52DecodedSignal d;
 CHECK(mblink_mercedes_egs52_frame_count()==121U);CHECK(mblink_mercedes_egs52_signal_count()==528U);CHECK(strcmp(mblink_mercedes_egs52_source_revision(),"1b96089660e97c91811b3d9cda6ca6f82b458c69")==0);
 CHECK(mblink_mercedes_egs52_frame_match_count(UINT32_C(0x218))==2U);fr=mblink_mercedes_egs52_frame_match_at(UINT32_C(0x218),0U);CHECK(fr!=NULL&&mblink_mercedes_egs52_signal_find(fr,"FPC_AAD")!=NULL&&mblink_mercedes_egs52_signal_find(fr,"I_IST_GET")==NULL);s=mblink_mercedes_egs52_signal_find(fr,"FPC_AAD");setp(b,s->bit_offset,s->bit_length,UINT64_C(1));CHECK(mblink_mercedes_egs52_decode_signal(s,b,8U,&d));CHECK(d.enum_available&&strcmp(d.enum_name,"KOMFORT")==0);fr=mblink_mercedes_egs52_frame_match_at(UINT32_C(0x218),1U);CHECK(fr!=NULL&&mblink_mercedes_egs52_signal_find(fr,"I_IST_GET")!=NULL);
 memset(b,0,sizeof(b));fr=mblink_mercedes_egs52_frame_match_at(UINT32_C(0x230),0U);s=mblink_mercedes_egs52_signal_find(fr,"WHC");setp(b,s->bit_offset,s->bit_length,UINT64_C(5));CHECK(mblink_mercedes_egs52_decode_signal(s,b,8U,&d));CHECK(d.enum_available&&strcmp(d.enum_name,"D")==0);
 memset(b,0,sizeof(b));fr=mblink_mercedes_egs52_frame_match_at(UINT32_C(0x408),0U);s=mblink_mercedes_egs52_signal_find(fr,"WRC");CHECK(s!=NULL&&s->masked&&s->mask==UINT64_C(135));b[7]=UINT8_C(0x80);CHECK(mblink_mercedes_egs52_decode_signal(s,b,8U,&d));CHECK(d.enum_available&&strcmp(d.enum_name,"BG180")==0);
 puts("Mercedes EGS52 isolated lookup tests passed");return 0;}

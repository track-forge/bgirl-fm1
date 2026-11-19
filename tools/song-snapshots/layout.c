/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../felucca/web/emu/felucca_web.c"
#include <stdio.h>
int main(void) {
    printf("{\"fm6Size\":%u,\"globals\":[",FM6_PACKED);
    for(unsigned i=0;i<G_COUNT;i++) printf("%s[%d,%d]",i?",":"",(i==G_SLOT||i==G_LOAD||i==G_SAVE)?GP[i].def:GP[i].min,(i==G_SLOT||i==G_LOAD||i==G_SAVE)?GP[i].def:GP[i].max);
    printf("],\"engines\":[");
    for(unsigned e=0;e<NENGINES;e++) {
        printf("%s{\"id\":%u,\"name\":\"%s\",\"parameters\":[",e?",":"",e,ENGINES[e]->name);
        for(unsigned i=0;i<P_COUNT;i++) { const param_desc_t *d=param_desc_of(e,i);
            printf("%s[%d,%d]",i?",":"",d->min,d->max);
        }
        printf("],\"forbidden\":{");
        unsigned comma=0;
        for(unsigned i=0;i<P_COUNT;i++) {
            const param_desc_t *d=param_desc_of(e,i); unsigned found=0;
            for(int v=d->min;v<=d->max;v++) if(param_fit(d,v)!=v) found++;
            if(!found)continue;
            printf("%s\"%u\":[",comma++?",":"",i);unsigned c=0;
            for(int v=d->min;v<=d->max;v++) if(param_fit(d,v)!=v) printf("%s%d",c++?",":"",v);
            printf("]");
        }
        printf("}}");
    }
    printf("],\"motionParameters\":["); unsigned c=0;
    for(unsigned i=0;i<P_COUNT;i++)if(motion_param(i))printf("%s%u",c++?",":"",i);
    puts("]}");
}

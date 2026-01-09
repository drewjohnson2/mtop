#include "../../include/text.h"
#include "../../include/helpers.h"

#define PRINT_TITLEFC(wd, y, x, fmt, val, pair) 	    \
    do {											    \
		if (*y >= wd->wHeight - 4) 					    \
    	{											    \
    	    *y = 4;									    \
    	    *x = wd->wWidth / 2;					    \
    	}											    \
													    \
	    PRINTFC(wd->window, (*y)++, *x, fmt, val, pair);\
    } while (0)

#define PRINT_VALUEFC(wd, y, x, fmt, val, padding, pair) 		\
    do {														\
		if (*y >= wd->wHeight - 4) 								\
    	{														\
    		*y = 4;												\
    		*x = wd->wWidth / 2;								\
    	}														\
																\
		u8 valuePos = padding + *x + 2;							\
																\
		PRINTFC(wd->window, (*y)++, valuePos, fmt, val, pair);	\
    } while (0)

void platform_print_fields(
    ProcessStatsViewData *vd,
    const WindowData *wd,
    const MT_Color_Pairs boxPair,
    u8 posX,
    u8 posY
)
{
    const u8 valuePaddingLeft = 10;

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY, posX, "%s", text(TXT_CPU_PCT_COL), MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, valuePaddingLeft + posX + 2, "%.2f", vd->cpuPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY, posX, "%s", text(TXT_MEM_PCT_COL), MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, valuePaddingLeft + posX + 2, "%.2f", vd->memPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    wattron(wd->window, A_BOLD);

    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_STATE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_THREADS), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_PPID), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMRSS), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSIZE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMLOCK), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMDATA), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSTACK), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSWAP), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMEXE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMLIB), MT_PAIR_PRC_STAT_NM);

    wattroff(wd->window, A_BOLD);

    posY = 6;

    PRINT_VALUEFC(wd, &posY, &posX, "%c", vd->state, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%d", vd->threads, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%hu", vd->ppid, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmRss, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmSize, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmLock, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmData, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmStack, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmSwap, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmExe, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmLib, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINTFC(wd->window, wd->wHeight - 2, 3, "%s", text(TXT_RET_LIST_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 2, 5, "%s", text(TXT_RET_LIST), MT_PAIR_CTRL_TXT);
    SET_COLOR(wd->window, boxPair);
}

void platform_set_vd(ProcessStatsViewData *vd, Process *cur)
{
    vd->vmLock = cur->vmLock;
    vd->vmData = cur->vmData;
    vd->vmStack = cur->vmStack;
    vd->vmSwap = cur->vmSwap;
    vd->vmExe = cur->vmExe;
    vd->vmLib = cur->vmLib;
}

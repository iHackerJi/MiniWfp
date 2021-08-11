#pragma once
#include <ntifs.h>
#include <fwpsk.h>
#include <fwpmk.h>
#include "Def.h"

#define	MiniWfpDevice		L"\\Device\\MiniWfp"

NTSTATUS	MiniWfpUnloadWfp(PDRIVER_OBJECT	pDriverObj);
NTSTATUS	MiniWfpInitMiniWfp(PDRIVER_OBJECT	pDriverObj);

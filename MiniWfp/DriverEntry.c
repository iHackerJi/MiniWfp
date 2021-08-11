#include <ntifs.h>
#include "MiniWfp.h"

void UnloadDriver(PDRIVER_OBJECT	pDriverObj) {
	MiniWfpUnloadWfp(pDriverObj);
}

NTSTATUS	DriverEntry(PDRIVER_OBJECT	pDriverObj,PUNICODE_STRING pReg) {

	pDriverObj->DriverUnload = UnloadDriver;
	NTSTATUS	Status;
	Status = MiniWfpInitMiniWfp(pDriverObj);
	return	Status;
}
#include "MiniWfp.h"

//DEFINE_GUID(WFP_CALLOUT_GXUID, 0x99133EAA, 0x634F, 0x0D18, 0x80, 0x2B, 0xE7, 0x84, 0xC0, 0x66, 0x71, 0xBF);

UINT32	MiniWfpRegCalloutId = 0;
UINT32	MiniWfpFwpmCalloutId = 0;
UINT64	MiniWfpFilter = 0;
HANDLE	EngineHandle=0;

PUCHAR PacketData = NULL;


NTSTATUS NTAPI MiniWfpNotifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER0* filter
) {

	return	STATUS_SUCCESS;

}


char* strstr_s(char *a, char *b, int Size)

{

	int i, j, alen = Size, blen = strlen(b);

	for (i = 0; i < alen - blen + 1; i++)

		if (a[i] == b[0]) {

			for (j = 1; j < blen; j++) if (a[i + j] != b[j])break;

			if (j == blen)
				return (char*)(a+i);

		}

	return (char*)-1;

}




void NTAPI  MiniWfpClassifyFn(
	const FWPS_INCOMING_VALUES0 *inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0 *inMetaValues,
	void *layerData,
	const FWPS_FILTER0 *filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT0 *classifyOut
)
{

	ULONG RemoteIPv4 = 0;
	ULONG LocalIPv4 = 0;

	USHORT RemotePort = 0;
	USHORT LocalPort = 0;

	 
	PNET_BUFFER_LIST  StreamPacket = layerData;


	PNET_BUFFER_LIST ListEntry = StreamPacket;
	PNET_BUFFER	BufferEntry = NULL; //NET_BUFFER_LIST_FIRST_NB(ListEntry)
	PUCHAR	Data = NULL;
	BOOLEAN	IsFlags = FALSE;

	RemotePort = inFixedValues->incomingValue[FWPS_FIELD_DATAGRAM_DATA_V4_IP_REMOTE_PORT].value.uint16;
	classifyOut->actionType = FWP_ACTION_PERMIT;
	if (RemotePort == 53) {

//拦截53端口
		if (ListEntry)
		{
			do
			{

				BufferEntry = NET_BUFFER_LIST_FIRST_NB(ListEntry);
				do
				{

					Data = NdisGetDataBuffer(BufferEntry, BufferEntry->DataLength, PacketData, 1, 0);
					if (Data || PacketData)
					{
						//DbgBreakPoint();
						if (strstr_s(PacketData, "baidu", BufferEntry->DataLength)!= (char*)-1)
						{
							classifyOut->actionType = FWP_ACTION_BLOCK;
							DbgPrint("Catch a Baidu Dog\r\n");
						}


						//DbgPrint("GetData:\r\n");
						//for (int Index = 0; Index < BufferEntry->DataLength; Index++) {
						//	DbgPrint("%02X ", PacketData[Index]);
						//}
						//DbgPrint("\n");
					}

					BufferEntry = BufferEntry->Next;
				} while (BufferEntry);



				ListEntry = NET_BUFFER_LIST_NEXT_NBL(StreamPacket);
			} while (ListEntry);//遍历PNET_BUFFER_LIST
		}
	
	}




}


void NTAPI MiniWfpFlowDeleteFn(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
) {

}

NTSTATUS	MiniWfpCreateDevice(PDRIVER_OBJECT	pDriverObj, PDEVICE_OBJECT	*pDeviceObj) {
	NTSTATUS	Status;
	UNICODE_STRING	MiniWfpDeviceString = { 0 };
	RtlInitUnicodeString(&MiniWfpDeviceString, MiniWfpDevice);


	return	IoCreateDevice(pDriverObj, 0, &MiniWfpDeviceString, FILE_DEVICE_UNKNOWN, 0, FALSE, pDeviceObj);
}

NTSTATUS	MiniWfpRegisterCallout(PDEVICE_OBJECT	pDeviceObj) {

	FWPS_CALLOUT0	FwpsCallout = { 0 };
	FwpsCallout.calloutKey = WFP_CALLOUT_GUID;
	FwpsCallout.classifyFn = MiniWfpClassifyFn;
	FwpsCallout.notifyFn = MiniWfpNotifyFn;
	FwpsCallout.flowDeleteFn = MiniWfpFlowDeleteFn;
	FwpsCallout.flags = 0;

	return	FwpsCalloutRegister0(pDeviceObj, &FwpsCallout, &MiniWfpRegCalloutId);
}

NTSTATUS	MiniWfpAddCalloutToEngine(HANDLE	EngineHandle) {
	FWPM_CALLOUT0	FwpmCallout = { 0 };
	FwpmCallout.calloutKey = WFP_CALLOUT_GUID;
	FwpmCallout.displayData.name = L"MiniWfpCallout";
	FwpmCallout.displayData.description = L"MiniWfpCallout v1.0";
	FwpmCallout.flags = 0;
	FwpmCallout.applicableLayer = FWPM_LAYER_DATAGRAM_DATA_V4;  //设置要依赖的分层

	return	FwpmCalloutAdd0(EngineHandle, &FwpmCallout, NULL, &MiniWfpFwpmCalloutId);
}

NTSTATUS	MiniWfpAddFilterToEngine(HANDLE	EngineHandle) {

	FWPM_FILTER0	FwpmFiler = { 0 };
	FwpmFiler.displayData.name = L"MiniWfpFilter";
	FwpmFiler.displayData.description = L"FwpmFiler.displayData v1.0";
	FwpmFiler.flags = FWPM_FILTER_FLAG_NONE;

	FwpmFiler.layerKey = FWPM_LAYER_DATAGRAM_DATA_V4;
	FwpmFiler.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;

	FwpmFiler.weight.type = FWP_EMPTY;
	FwpmFiler.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	FwpmFiler.action.calloutKey = WFP_CALLOUT_GUID;


	return FwpmFilterAdd0(EngineHandle, &FwpmFiler, NULL, &MiniWfpFilter);
}

NTSTATUS	MiniWfpUnloadWfp(PDRIVER_OBJECT	pDriverObj) {
	NTSTATUS	Status = STATUS_SUCCESS;

	if (pDriverObj->DeviceObject)
		IoDeleteDevice(pDriverObj->DeviceObject);
	
	if (MiniWfpRegCalloutId)
		Status = FwpsCalloutUnregisterById0(MiniWfpRegCalloutId);

	if (MiniWfpFilter)
		Status = FwpmFilterDeleteById0(EngineHandle, MiniWfpFilter);

	if (MiniWfpFwpmCalloutId)
		Status = FwpmCalloutDeleteById0(EngineHandle, MiniWfpFwpmCalloutId);

	//if (EngineHandle)
		//Status = ZwClose(EngineHandle); //不必关句柄，这里会显示句柄无效

	ExFreePoolWithTag(PacketData, 'MWFP');
	
	if (!NT_SUCCESS(Status))	DbgBreakPoint();

	return	Status;

}

NTSTATUS	MiniWfpInitMiniWfp(PDRIVER_OBJECT	pDriverObj) {
	PDEVICE_OBJECT	pDeviceObj = NULL;
	NTSTATUS	Status;


//1.	创建设备
	Status = MiniWfpCreateDevice(pDriverObj, &pDeviceObj);
	if (!NT_SUCCESS(Status))	goto	_Exit;
	
//2.	向设备注册一个呼出接口
	Status = MiniWfpRegisterCallout(pDeviceObj);
	if (!NT_SUCCESS(Status))	goto	_Exit;

//3.	向过滤引擎添加呼出接口
	//打开引擎
	Status = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &EngineHandle);
	if (!NT_SUCCESS(Status))	goto	_Exit;

	//添加呼出接口
	Status = MiniWfpAddCalloutToEngine(EngineHandle);
	if (!NT_SUCCESS(Status))	goto	_Exit;

//4.	添加过滤器

	Status = MiniWfpAddFilterToEngine(EngineHandle);
	if (!NT_SUCCESS(Status))	goto	_Exit;

	PacketData = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE * 100,'mwfp');

	return	STATUS_SUCCESS;

_Exit:
	MiniWfpUnloadWfp(pDriverObj);
	DbgBreakPoint();
	return	STATUS_UNSUCCESSFUL;

}
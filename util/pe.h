#pragma once

// dumped from windows 10 x64.

struct PEB_LDR_DATA {
	uint32_t		Length;
	uint8_t			Initialized;
	uintptr_t		SsHandle;
	LIST_ENTRY		InLoadOrderModuleList;
	LIST_ENTRY		InMemoryOrderModuleList;
	LIST_ENTRY		InInitializationOrderModuleList;
	uintptr_t		EntryInProgress;
	uint8_t			ShutdownInProgress;
	uintptr_t		ShutdownThreadId;
};

struct UNICODE_STRING {
	uint16_t	Length;
	uint16_t	MaximumLength;
	wchar_t		*Buffer;
};

struct STRING {
	uint16_t	Length;
	uint16_t	MaximumLength;
	char		*Buffer;
};

struct CURDIR {
	UNICODE_STRING	DosPath;
	uintptr_t		Handle;
};

struct RTL_DRIVE_LETTER_CURDIR {
	uint16_t	Flags;
	uint16_t	Length;
	uint32_t	TimeStamp;
	STRING		DosPath;
};

struct RTL_USER_PROCESS_PARAMETERS {
	uint32_t					MaximumLength;
	uint32_t					Length;
	uint32_t					Flags;
	uint32_t					DebugFlags;
	uintptr_t					ConsoleHandle;
	uint32_t					ConsoleFlags;
	uintptr_t					StandardInput;
	uintptr_t					StandardOutput;
	uintptr_t					StandardError;
	CURDIR						CurrentDirectory;
	UNICODE_STRING				DllPath;
	UNICODE_STRING				ImagePathName;
	UNICODE_STRING				CommandLine;
	uintptr_t					Environment;
	uint32_t					StartingX;
	uint32_t					StartingY;
	uint32_t					CountX;
	uint32_t					CountY;
	uint32_t					CountCharsX;
	uint32_t					CountCharsY;
	uint32_t					FillAttribute;
	uint32_t					WindowFlags;
	uint32_t					ShowWindowFlags;
	UNICODE_STRING				WindowTitle;
	UNICODE_STRING				DesktopInfo;
	UNICODE_STRING				ShellInfo;
	UNICODE_STRING				RuntimeData;
	RTL_DRIVE_LETTER_CURDIR		CurrentDirectores[ 32 ];
	uintptr_t					EnvironmentSize;
	uintptr_t					EnvironmentVersion;
	uintptr_t					PackageDependencyData;
	uint32_t					ProcessGroupId;
	uint32_t					LoaderThreads;
};

struct RTL_BALANCED_NODE {
	RTL_BALANCED_NODE	*Children[ 2 ];
	RTL_BALANCED_NODE	*Left;
	RTL_BALANCED_NODE	*Right;
	uintptr_t			ParentValue;
};

struct _PEB {
	uint8_t							InheritedAddressSpace;
	uint8_t							ReadImageFileExecOptions;
	uint8_t							BeingDebugged;
	uint8_t							BitField;
	//uchar							Padding0[ 4 ];
	uintptr_t						Mutant;
	uintptr_t						ImageBaseAddress;
	PEB_LDR_DATA					*Ldr;
	RTL_USER_PROCESS_PARAMETERS		*ProcessParameters;
	uintptr_t						SubSystemData;
	uintptr_t						ProcessHeap;
	RTL_CRITICAL_SECTION			*FastPebLock;
	uintptr_t						AtlThunkSListPtr;
	uintptr_t						IFEOKey;
	uint32_t						CrossProcessFlags;
	uint8_t							Padding1[ 4 ];
	uintptr_t						KernelCallbackTable;
	uintptr_t						UserSharedInfoPtr;
	uint32_t						SystemReserved[ 1 ];
	uint32_t						AtlThunkSListPtr32;
	uintptr_t						ApiSetMap;
	uint32_t						TlsExpansionCounter;
	uint8_t							Padding2[ 4 ];
	uintptr_t						TlsBitmap;
	uint32_t						TlsBitmapBits[ 2 ];
	uintptr_t						ReadOnlySharedMemoryBase;
	uintptr_t						SparePvoid0;
	uintptr_t						ReadOnlyStaticServerData;
	uintptr_t						AnsiCodePageData;
	uintptr_t						OemCodePageData;
	uintptr_t						UnicodeCaseTableData;
	uint32_t						NumberOfProcessors;
	uint32_t						NtGlobalFlag;
	LARGE_INTEGER					CriticalSectionTimeout;
	uintptr_t						HeapSegmentReserve;
	uintptr_t						HeapSegmentCommit;
	uintptr_t						HeapDeCommitTotalFreeThreshold;
	uintptr_t						HeapDeCommitFreeBlockThreshold;
	uint32_t						NumberOfHeaps;
	uint32_t						MaximumNumberOfHeaps;
	uintptr_t						ProcessHeaps;
	uintptr_t						GdiSharedHandleTable;
	uintptr_t						ProcessStarterHelper;
	uint32_t						GdiDCAttributeList;
	uint8_t							Padding3[ 4 ];
	RTL_CRITICAL_SECTION			*LoaderLock;
	uint32_t						OSMajorVersion;
	uint32_t						OSMinorVersion;
	uint16_t						OSBuildNumber;
	uint16_t						OSCSDVersion;
	uint32_t						OSPlatformId;
	uint32_t						ImageSubsystem;
	uint32_t						ImageSubsystemMajorVersion;
	uint32_t						ImageSubsystemMinorVersion;
	uint8_t							Padding4[ 4 ];
	uintptr_t						ActiveProcessAffinityMask;
#ifdef _WIN32
	uint32_t						GdiHandleBuffer[ 34 ];
#else
	uint32_t						GdiHandleBuffer[ 60 ];
#endif
	uintptr_t						PostProcessInitRoutine;
	uintptr_t						TlsExpansionBitmap;
	uint32_t						TlsExpansionBitmapBits[ 32 ];
	uint32_t						SessionId;
	uint8_t							Padding5[ 4 ];
	ULARGE_INTEGER					AppCompatFlags;
	ULARGE_INTEGER					AppCompatFlagsUser;
	uintptr_t						pShimData;
	uintptr_t						AppCompatInfo;
	UNICODE_STRING					CSDVersion;
	uintptr_t						ActivationContextData;
	uintptr_t						ProcessAssemblyStorageMap;
	uintptr_t						SystemDefaultActivationContextData;
	uintptr_t						SystemAssemblyStorageMap;
	uintptr_t						MinimumStackCommit;
	uintptr_t						FlsCallback;
	LIST_ENTRY						FlsListHead;
	uintptr_t						FlsBitmap;
	uint32_t						FlsBitmapBits[ 4 ];
	uint32_t						FlsHighIndex;
	uintptr_t						WerRegistrationData;
	uintptr_t						WerShipAssertPtr;
	uintptr_t						pUnused;
	uintptr_t						pImageHeaderHash;
	uint32_t						TracingFlags;
	uint8_t							Padding6[ 4 ];
	uint64_t						CsrServerReadOnlySharedMemoryBase;
	uintptr_t						TppWorkerpListLock;
	LIST_ENTRY						TppWorkerpList;
	uintptr_t						WaitOnAddressHashTable[ 128 ];
};

struct LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY				InLoadOrderLinks;
	LIST_ENTRY				InMemoryOrderLinks;
	LIST_ENTRY				InInitializationOrderLinks;
	uintptr_t				DllBase;
	uintptr_t				EntryPoint;
	uint32_t				SizeOfImage;
	UNICODE_STRING			FullDllName;
	UNICODE_STRING			BaseDllName;
	uint8_t					FlagGroup[ 4 ];
	uint32_t				Flags;
	uint16_t				ObsoleteLoadCount;
	uint16_t				TlsIndex;
	LIST_ENTRY				HashLinks;
	uint32_t				TimeDateStamp;
	uintptr_t				EntryPointActivationContext;
	uintptr_t				Lock;
	uintptr_t				DdagNode;
	LIST_ENTRY				NodeModuleLink;
	uintptr_t				LoadContext;
	uintptr_t				ParentDllBase;
	uintptr_t				SwitchBackContext;
	RTL_BALANCED_NODE		BaseAddressIndexNode;
	RTL_BALANCED_NODE		MappingInfoIndexNode;
	uintptr_t				OriginalBase;
	LARGE_INTEGER			LoadTime;
	uint32_t				BaseNameHashValue;
	uint32_t				LoadReason;
	uint32_t				ImplicitPathOptions;
	uint32_t				ReferenceCount;
};
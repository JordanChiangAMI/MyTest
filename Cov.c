


extern EFI_SMM_SYSTEM_TABLE2           *pSmst2;

typedef struct {
    UINT8           ApicId;
    BOOLEAN         IsThrottleEnabled;
    UINT8           Modulation;
    UINT8           Reserved;
} THROTTLING_CPU_DATA;

typedef
EFI_STATUS
(EFIAPI *THROTTLING_VALIDATE_MODULATION) (
    UINT8                                   Modulation,        // Check this Modulation
    BOOLEAN                                 *Valid,            // If the Modulation is valid
    UINT8                                   *MinModulation,    // Minimum settings for Modulation value
    UINT8                                   *MaxModulation     // Maximum settings for Modulation value
    );

typedef
EFI_STATUS
(EFIAPI *THROTTLING_SET_THROTTLE) (
    THROTTLING_CONTROL_PROTOCOL        *This,
    THROTTLING_CPU_DATA                *CpuData,
    UINTN                                   NumCpuData
    );

typedef
EFI_STATUS
(EFIAPI *THROTTLING_GET_THROTTLE) (
    THROTTLING_CONTROL_PROTOCOL        *This,
    THROTTLING_CPU_DATA                *CpuData,
    UINTN                                   NumCpuData
    );

typedef struct _THROTTLING_CONTROL_PROTOCOL {
    UINTN                                   NumberOfCpus;
    THROTTLING_VALIDATE_MODULATION     ValidateModulation;
    THROTTLING_SET_THROTTLE            GetThrottle;
    THROTTLING_GET_THROTTLE            SetThrottle;
} THROTTLING_CONTROL_PROTOCOL;

typedef struct {
    THROTTLING_CPU_DATA   *CpuData;
    UINTN                      NumCpuData;
    UINTN                      CpuIndex;
    BOOLEAN                    IsGet;
    UINT64                      Signature;
} THROTTLING_PRIVATE_DATA;


EFI_STATUS
EFIAPI
SetThrottle (
    THROTTLING_CONTROL_PROTOCOL *This,
    THROTTLING_CPU_DATA         *CpuData,
    UINTN                            NumCpuData
    )
{
    UINTN                   Index;
    THROTTLING_PRIVATE_DATA *Private;
    EFI_STATUS              Status;
    UINTN RunningCpuNum;

    RunningCpuNum = pSmst2->CurrentlyExecutingCpu;

    Status = AllocatePool(EfiRuntimeServicesData,
                                         sizeof(THROTTLING_PRIVATE_DATA)*NumCpuData,
                                         (VOID **) &Private );
    if(EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
    }
		  
    for (Index = 0; Index < NumCpuData; Index++)
    {
        (Private + Index)->CpuData    = CpuData;
        (Private + Index)->NumCpuData = NumCpuData;
        (Private + Index)->IsGet      = FALSE;
        (Private + Index)->CpuIndex = Index;
        
        if(RunningCpuNum == Index )
        {
            continue;
        }//if(RunningCpuNum ==Index )
		
        Status = pSmst2->SmmStartupThisAp (CpuThrottle, Index, (Private + Index));
     }//for (Index = 0; Index < NumCpuData; Index++)
         
    (Private + Index)->CpuIndex = RunningCpuNum;

    Status = FreePool (Private);
    if (EFI_ERROR (Status)){
        ASSERT_EFI_ERROR (Status);
        return Status;
    }
    return EFI_SUCCESS;
}

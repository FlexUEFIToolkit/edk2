#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include "FlexUEFIToolkitDxe.h"
#include "QemuFlash.h"

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include "Guid/GlobalVariable.h"
#include <Library/MemoryAllocationLib.h>
typedef struct{
  UINT16 bootnum;
  UINTN show_verbose : 1;
  UINTN set_bootorder : 1;
  UINTN  : 1;
  UINTN usage : 1;
}BOOTMGROPTS;

BOOTMGROPTS opts={0};

BOOLEAN
EFIAPI
IsCharDigit (
  IN CHAR16  Char
  )
{
  return (BOOLEAN) (Char >= L'0' && Char <= L'9')||(Char >= L'A' && Char <= L'F');
}

char input[12]={0};
//refer to IntelFrameworkPkg/Lib/GenericBdsLib/BdsMisc.c BdsLibGetVariableAndSize
VOID *
EFIAPI
mGetVariable(
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize,
  OUT UINT32              *Attr
  )
{
    EFI_STATUS  Status;
    UINTN       BufferSize;
    VOID        *Buffer;

    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: Order %04x\n \n",input));

    Buffer = NULL;
    //
    // Pass in a zero size buffer to find the required buffer size.
    //
    BufferSize  = 0;
    Status      = gRT->GetVariable (Name, VendorGuid, Attr, &BufferSize, Buffer);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: BufferSize %04x\n",BufferSize));
        //
        // Read variable
        //
        Status = gRT->GetVariable (Name, VendorGuid, Attr, &BufferSize, input);
    }
    *VariableSize = BufferSize;
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: BufferSize %04x\n",BufferSize));
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: Status %04x\n \n",Status));
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: Order %04x\n \n",input));
    //   return Buffer;
    return input;
}

VOID
ParseOpt(
	UINTN Argc, 
	CHAR16 **Argv
	)
{
  UINTN Index;

  for (Index = 1; Index < Argc; Index ++) {
    if ((Argv[Index][0] != L'-') || (Argv[Index][2] != L'\0')) {
      return ;
    }

    switch (Argv[Index][1]) {
    case L'v':
      opts.show_verbose = 1;
      break;
    case L's':
      opts.set_bootorder = 1;
      opts.bootnum = (UINT16)StrHexToUintn(Argv[Index+1]);
      break;
    case L'h':
    case L'H':
    case L'?':
      opts.usage = 1;
      break;

    default:
      break;
    }
  }
}

EFI_STATUS
EFIAPI
FlexUEFISetFirstBoot (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: futSetFisrtBoot test0.\n \n"));
    EFI_STATUS    Status;
    UINT16        *BootVariable;
    UINTN         BootVariableSize;
    UINTN         NewNameSize;
    UINTN         NameSize;
    UINTN         i;
    UINT32        Attr;
    EFI_GUID      VarGuid;

    BootVariable = mGetVariable(L"BootOrder", &gEfiGlobalVariableGuid, &BootVariableSize, &Attr);
    if (BootVariable != NULL){
        DEBUG((DEBUG_INFO, "BootOrder:  "));
        for(i=0; i<(BootVariableSize/2); i++){
            DEBUG((DEBUG_INFO, " %04x ",BootVariable[i]));
        }
        DEBUG((DEBUG_INFO, "\n"));
    }
    
    // Set BootOrder
    opts.set_bootorder=1;
    if (opts.set_bootorder){
        BootVariable = mGetVariable(L"BootOrder", &gEfiGlobalVariableGuid, &BootVariableSize, &Attr);
        char order[12] = {05,00,06,00,03,00,01,00,02,00,04,00};
        Status = gRT->SetVariable(L"BootOrder", &gEfiGlobalVariableGuid, Attr, BootVariableSize, order);
    }

    //get BootCurrent
    BootVariable = mGetVariable(L"BootCurrent", &gEfiGlobalVariableGuid, &BootVariableSize, NULL);
    if (BootVariable != NULL){
        DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: BootCurrent:  %04x\n \n",*BootVariable));
    }
    //get BootOrder
    BootVariable = mGetVariable(L"BootOrder", &gEfiGlobalVariableGuid, &BootVariableSize, &Attr);
    if (BootVariable != NULL){
        DEBUG((DEBUG_INFO, "BootOrder:  "));
        for(i=0; i<(BootVariableSize/2); i++){
            DEBUG((DEBUG_INFO, " %04x ",BootVariable[i]));
        }
        DEBUG((DEBUG_INFO, "\n"));
        return Status;
    }
    return Status;
}

int
EFIAPI
FlexUEFIToLowerCase(
    IN  char *Src,
    OUT char *Dst
)
{
    int k = 0;
    while(Src[k] != '\0') {
        if (Src[k] >= 'A' && Src[k] <= 'Z') {
            Dst[k] = (char)((int)Src[k] - (int)'A' + (int)'a');
        } else {
            Dst[k] = Src[k];
        }
        ++k;
    }
    Dst[k] = '\0';
    return k;
}

int
EFIAPI
FlexUEFIReadFlash(
  IN        EFI_LBA  Lba,
  IN        UINTN    Offset,
  IN        UINTN    *NumBytes,
  OUT       UINT8    *Buffer
)
{
    EFI_STATUS status = QemuFlashRead(Lba,Offset,NumBytes,Buffer);
    Buffer[(*NumBytes)-1]='\0';
    if((int)status < 0) 
            DEBUG((DEBUG_ERROR, "QemuFlashRead error: %d\n",(int)status));
    return (int)status;
}

EFI_STATUS
EFIAPI
FlexUEFIChangeBIOS(
  IN        EFI_LBA  Lba,
  IN        UINTN    Offset,
  IN        UINTN    *NumBytes,
  IN        UINT8    *Buffer
)
{
    // TODO
    EFI_STATUS status = QemuFlashWrite(Lba, Offset, NumBytes, Buffer);
    if((int)status < 0)
        DEBUG((DEBUG_ERROR, "QemuFlashWrite error: %d\n",(int)status));
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FlexUefiToolkitFunc (
  IN  UINT32  Args0,
  IN  UINT8  *Args1,
  OUT UINT8  *Args2
)
{
    EFI_STATUS status;
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: FlexUefiToolkitFunc: %d\n", Args0));
    switch((FLEX_UEFI_TOOKLIT_FUNC_TYPE)Args0) {
        case futNonOp:
            status = EFI_SUCCESS;
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: futNonOp."));
            break;
        case futToLowerCase:
            status = FlexUEFIToLowerCase((char *)Args1, (char *)Args2);
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: futToLowerCase, src = %s, dst = %s.", (char *)Args1, (char *)Args2));
            break;
        case futReadFlash:
        {
            UINTN lba,offset,readByte;
            lba = ((char *)Args1)[2]-'0';
            offset = ((char *)Args1)[4]-'0';
            readByte = AsciiStrDecimalToUint64(((char *)Args1)+6);
            DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: futReadFlash readByte: %d\n", readByte));
            // char buffer[readByte];
            status = FlexUEFIReadFlash(lba,offset,&readByte,(UINT8 *)Args2);
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: futReadFlash, lba = %lld, offset = %lld, readByte = %lld.",lba,offset,readByte));
            break;
        }
        case futChangeBIOS:
        {
            UINTN lba, offset, writeByte;
            lba = ((UINTN*)(Args1+4))[0];
            offset = ((UINTN*)(Args1+4))[1];
            writeByte = ((UINTN*)(Args1+4))[2];
            DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: futChangeBIOS writeByte: %d\n", writeByte));
            status = FlexUEFIChangeBIOS(lba, offset, &writeByte, (UINT8 *)Args1 + 28);
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: futChangeBIOS, lba = %lld, offset = %lld, writeByte = %lld.\n",lba,offset,writeByte));
            break;
        }
        case futSetFisrtBoot:
            DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: futSetFisrtBoot set BOOTXXXX First to Boot. BEFORE\n \n"));
            status = FlexUEFISetFirstBoot(1, (CHAR16 **)Args1);
            DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe :: futSetFisrtBoot set BOOTXXXX First to Boot. AFTER\n \n"));
            break;
        default:
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: Unknown Func Type."));
            status = 0x12345;
            break;
    }
    return status;
}

EFI_STATUS
EFIAPI
FlexUEFIToolkitDxeEntryPoint(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    gRT->FlexUefiToolkitFunc = FlexUefiToolkitFunc;
    DEBUG((DEBUG_INFO, "FlexUEFIToolkitDxe executed EntryPoint() success.\n"));

    char var_data[] = "0123456789\n";
    EFI_STATUS e;

    DEBUG((DEBUG_INFO,"Creation of %s variable\n", FLEX_UEFI_TOOLKIT_VARIABLE_NAME));

    e = gRT->SetVariable(FLEX_UEFI_TOOLKIT_VARIABLE_NAME, &gTestVarGuid, EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, sizeof(var_data), (VOID *)var_data);
    if(!EFI_ERROR(e)) {
        DEBUG((DEBUG_INFO,"New UEFI Variable %s creation success.\n", FLEX_UEFI_TOOLKIT_VARIABLE_NAME));
    } else {
        
        DEBUG((DEBUG_INFO,"UEFI Variable %s creation failure: %x.\n", FLEX_UEFI_TOOLKIT_VARIABLE_NAME, e));
    }

    UINTN ret = (INTN)(RETURN_STATUS)QemuFlashInitialize ();
    if (EFI_ERROR (QemuFlashInitialize ())) {
        // Return an error so image will be unloaded
        DEBUG((DEBUG_ERROR,"FlexUEFIToolkitDxeEntryPoint::QEMU flash was not detected. Writable FVB is not being installed. Error Num: %d\n",ret));
    }
      

    return EFI_SUCCESS;
}
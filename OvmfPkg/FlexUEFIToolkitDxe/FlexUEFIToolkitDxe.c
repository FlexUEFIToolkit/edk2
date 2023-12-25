#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include "FlexUEFIToolkitDxe.h"
#include "QemuFlash.h"

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
    IN  char *Src,
    OUT char *Dst
)
{
    // TODO
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
    Args0 = ((char *)Args1)[0]-'0';
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
            char buffer[readByte];
            status = FlexUEFIReadFlash(lba,offset,&readByte,(UINT8 *)buffer);
            DEBUG((DEBUG_ERROR, "FlexUEFIToolkitDxe :: futReadFlash, lba = %lld, offset = %lld, readByte = %lld, content = %s.",lba,offset,readByte,buffer));
            break;
        }
        case futChangeBIOS:
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
      

    return EFI_SUCCESS;
}
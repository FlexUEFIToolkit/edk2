#ifndef FLEX_UEFI_TOOLKIT_DXE_H_
#define FLEX_UEFI_TOOLKIT_DXE_H_

#define FLEX_UEFI_TOOLKIT_VARIABLE_NAME L"FlexUEFIToolkit"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    futNonOp            = 0,
    futToLowerCase      = 1,
    futReadFlash        = 2,
    futChangeBIOS	      = 3,
    futSetFisrtBoot     = 4,
    futFuncNum

} FLEX_UEFI_TOOKLIT_FUNC_TYPE;


#ifdef __cplusplus
}
#endif
#endif

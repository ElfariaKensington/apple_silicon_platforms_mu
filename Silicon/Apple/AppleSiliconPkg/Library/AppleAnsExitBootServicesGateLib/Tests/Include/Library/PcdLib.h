#include <Uefi.h>
extern UINT64 gTestPreloadBase;
extern UINT32 gTestPreloadSize;
extern UINT64 gTestEspStart, gTestEspBlocks;
extern UINT8 gTestEspGuid[16];
#define PcdGet64(Name) ANS_TEST_PCD_##Name
#define PcdGet32(Name) ANS_TEST_PCD_##Name
#define PcdGetPtr(Name) ANS_TEST_PCD_##Name
#define ANS_TEST_PCD_PcdPreloadedRamdiskBase gTestPreloadBase
#define ANS_TEST_PCD_PcdPreloadedRamdiskMaxSize gTestPreloadSize
#define ANS_TEST_PCD_PcdAnsInstalledEspStartLba gTestEspStart
#define ANS_TEST_PCD_PcdAnsInstalledEspBlocks gTestEspBlocks
#define ANS_TEST_PCD_PcdAnsInstalledEspGuid ((VOID *)gTestEspGuid)

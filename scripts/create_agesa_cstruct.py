from dissect import cstruct

TPS = """
enum AGESA_TP {
  StartProcessorTestPoints,           ///< 00 Entry used for range testing for @b Processor related TPs

  // Memory test points
  TpProcMemBeforeMemDataInit,         ///< 01 .. Memory structure initialization (Public interface)
  TpProcMemBeforeSpdProcessing,       ///< 02 .. SPD Data processing  (Public interface)
  TpProcMemAmdMemAuto,                ///< 03 .. Memory configuration  (Public interface)
  TpProcMemDramInit,                  ///< 04 .. DRAM initialization
  TpProcMemSPDChecking,               ///< 05 ..
  TpProcMemModeChecking,              ///< 06 ..
  TpProcMemSpeedTclConfig,            ///< 07 .. Speed and TCL configuration
  TpProcMemSpdTiming,                 ///< 08 ..
  TpProcMemDramMapping,               ///< 09 ..
  TpProcMemPlatformSpecificConfig,    ///< 0A ..
  TPProcMemPhyCompensation,           ///< 0B ..
  TpProcMemStartDcts,                 ///< 0C ..
  TpProcMemBeforeDramInit,            ///< 0D .. (Public interface)
  TpProcMemPhyFenceTraining,          ///< 0E ..
  TpProcMemSynchronizeDcts,           ///< 0F ..
  TpProcMemSystemMemoryMapping,       ///< 10 ..
  TpProcMemMtrrConfiguration,         ///< 11 ..
  TpProcMemDramTraining,              ///< 12 ..
  TpProcMemBeforeAnyTraining,         ///< 13 .. (Public interface)
  TpProcMemWriteLevelizationTraining, ///< 14 ..
  TpProcMemWlFirstPass,               ///< 15 .. Below 800Mhz first pass start
  TpProcMemWlSecondPass,              ///< 16 .. Above 800Mhz second pass start
  TpProcMemWlTrainTargetDimm,         ///< 17 .. Target DIMM configured
  TpProcMemWlPrepDimms,               ///< 18 ..  Prepare DIMMS for WL
  TpProcMemWlConfigDimms,             ///< 19 ..  Configure DIMMS for WL
  TpProcMemReceiverEnableTraining,    ///< 1A ..
  TpProcMemRcvrStartSweep,            ///< 1B .. Start sweep loop
  TpProcMemRcvrSetDelay,              ///< 1C .. Set receiver Delay
  TpProcMemRcvrWritePattern,          ///< 1D .. Write test pattern
  TpProcMemRcvrReadPattern,           ///< 1E .. Read test pattern
  TpProcMemRcvrTestPattern,           ///< 1F .. Compare test pattern
  TpProcMemRcvrCalcLatency,           ///< 20 .. Calculate MaxRdLatency per channel
  TpProcMemReceiveDqsTraining,        ///< 21 ..
  TpProcMemRcvDqsSetDelay,            ///< 22 .. Set Write Data delay
  TpProcMemRcvDqsWritePattern,        ///< 23 .. Write test pattern
  TpProcMemRcvDqsStartSweep,          ///< 24 .. Start read sweep
  TpProcMemRcvDqsSetRcvDelay,         ///< 25 .. Set Receive DQS delay
  TpProcMemRcvDqsReadPattern,         ///< 26 .. Read Test pattern
  TpProcMemRcvDqsTstPattern,          ///< 27 .. Compare Test pattern
  TpProcMemRcvDqsResults,             ///< 28 .. Update results
  TpProcMemRcvDqsFindWindow,          ///< 29 .. Start Find passing window
  TpProcMemTransmitDqsTraining,       ///< 2A ..
  TpProcMemTxDqStartSweep,            ///< 2B .. Start write sweep
  TpProcMemTxDqSetDelay,              ///< 2C .. Set Transmit DQ delay
  TpProcMemTxDqWritePattern,          ///< 2D .. Write test pattern
  TpProcMemTxDqReadPattern,           ///< 2E .. Read Test pattern
  TpProcMemTxDqTestPattern,           ///< 2F .. Compare Test pattern
  TpProcMemTxDqResults,               ///< 30 .. Update results
  TpProcMemTxDqFindWindow,            ///< 31 .. Start Find passing window
  TpProcMemMaxRdLatencyTraining,      ///< 32 ..
  TpProcMemMaxRdLatStartSweep,        ///< 33 .. Start sweep
  TpProcMemMaxRdLatSetDelay,          ///< 34 .. Set delay
  TpProcMemMaxRdLatWritePattern,      ///< 35 .. Write test pattern
  TpProcMemMaxRdLatReadPattern,       ///< 36 .. Read Test pattern
  TpProcMemMaxRdLatTestPattern,       ///< 37 .. Compare Test pattern
  TpProcMemOnlineSpareInit,           ///< 38 .. Online Spare init
  TpProcMemBankInterleaveInit,        ///< 39 .. Bank Interleave Init
  TpProcMemNodeInterleaveInit,        ///< 3A .. Node Interleave Init
  TpProcMemChannelInterleaveInit,     ///< 3B .. Channel Interleave Init
  TpProcMemEccInitialization,         ///< 3C .. ECC initialization
  TpProcMemPlatformSpecificInit,      ///< 3D .. Platform Specific Init
  TpProcMemBeforeAgesaReadSpd,        ///< 3E .. Before callout for "AgesaReadSpd"
  TpProcMemAfterAgesaReadSpd,         ///< 3F .. After callout for "AgesaReadSpd"
  TpProcMemBeforeAgesaHookBeforeDramInit,    ///< 40 .. Before optional callout "AgesaHookBeforeDramInit"
  TpProcMemAfterAgesaHookBeforeDramInit,     ///< 41 .. After optional callout "AgesaHookBeforeDramInit"
  TpProcMemBeforeAgesaHookBeforeDQSTraining, ///< 42 .. Before optional callout "AgesaHookBeforeDQSTraining"
  TpProcMemAfterAgesaHookBeforeDQSTraining,  ///< 43 .. After optional callout "AgesaHookBeforeDQSTraining"
  TpProcMemBeforeAgesaHookBeforeExitSelfRef, ///< 44 .. Before optional callout "AgesaHookBeforeDramInit"
  TpProcMemAfterAgesaHookBeforeExitSelfRef,  ///< 45 .. After optional callout "AgesaHookBeforeDramInit"
  TpProcMemAfterMemDataInit,          ///< 46 .. After MemDataInit
  TpProcMemInitializeMCT,             ///< 47 .. Before InitializeMCT
  TpProcMemLvDdr3,                    ///< 48 .. Before LV DDR3
  TpProcMemInitMCT,                   ///< 49 .. Before InitMCT
  TpProcMemOtherTiming,               ///< 4A.. Before OtherTiming
  TpProcMemUMAMemTyping,              ///< 4B .. Before UMAMemTyping
  TpProcMemSetDqsEccTmgs,             ///< 4C .. Before SetDqsEccTmgs
  TpProcMemMemClr,                    ///< 4D .. Before MemClr
  TpProcMemOnDimmThermal,             ///< 4E .. Before On DIMM Thermal
  TpProcMemDmi,                       ///< 4F .. Before DMI
  TpProcMemEnd,                       ///< 50 .. End of memory code

  // CPU test points
  TpProcCpuEntryDmi,                  ///< 51 .. Entry point CreateDmiRecords
  TpProcCpuEntryPstate,               ///< 52 .. Entry point GenerateSsdt
  TpProcCpuEntryPstateLeveling,       ///< 53 .. Entry point PStateLeveling
  TpProcCpuEntryPstateGather,         ///< 54 .. Entry point PStateGatherData
  TpProcCpuEntryWhea,                 ///< 55 .. Entry point CreateAcpiWhea
  TpProcCpuEntrySrat,                 ///< 56 .. Entry point CreateAcpiSrat
  TpProcCpuEntrySlit,                 ///< 57 .. Entry point CreateAcpiSlit
  TpProcCpuProcessRegisterTables,     ///< 58 .. Register table processing
  TpProcCpuSetBrandID,                ///< 59 .. Set brand ID
  TpProcCpuLocalApicInit,             ///< 5A .. Initialize local APIC
  TpProcCpuLoadUcode,                 ///< 5B .. Load microcode patch
  TpProcCpuBeforePMFeatureInit,       ///< 5C .. BeforePM feature dispatch point
  TpProcCpuPowerMgmtInit,             ///< 5D .. Power Management table processing
  TpProcCpuEarlyFeatureInit,          ///< 5E .. Early feature dispatch point
  TpProcCpuCoreLeveling,              ///< 5F .. Core Leveling
  TpProcCpuApMtrrSync,                ///< 60 .. AP MTRR sync up
  TpProcCpuPostFeatureInit,           ///< 61 .. POST feature dispatch point
  TpProcCpuFeatureLeveling,           ///< 62 .. CPU Feature Leveling
  TpProcCpuBeforeRelinquishAPsFeatureInit,   ///< 63 .. Before Relinquishing control of APs feature dispatch point
  TpProcCpuBeforeAllocateWheaBuffer,  ///< 64 .. Before the WHEA init code calls out to allocate a buffer
  TpProcCpuAfterAllocateWheaBuffer,   ///< 65 .. After the WHEA init code calls out to allocate a buffer
  TpProcCpuBeforeAllocateSratBuffer,  ///< 66 .. Before the SRAT init code calls out to allocate a buffer
  TpProcCpuAfterAllocateSratBuffer,   ///< 67 .. After the SRAT init code calls out to allocate a buffer
  TpProcCpuBeforeLocateSsdtBuffer,    ///< 68 .. Before the P-state init code calls out to locate a buffer
  TpProcCpuAfterLocateSsdtBuffer,     ///< 69 .. After the P-state init code calls out to locate a buffer
  TpProcCpuBeforeAllocateSsdtBuffer,  ///< 6A .. Before the P-state init code calls out to allocate a buffer
  TpProcCpuAfterAllocateSsdtBuffer,   ///< 6B .. After the P-state init code calls out to allocate a buffer

  // HT test points
  TpProcHtEntry = 0x71,               ///< 71 .. Coherent Discovery begin (Public interface)
  TpProcHtTopology,                   ///< 72 .. Topology match, routing, begin
  TpProcHtManualNc,                   ///< 73 .. Manual Non-coherent Init begin
  TpProcHtAutoNc,                     ///< 74 .. Automatic Non-coherent init begin
  TpProcHtOptGather,                  ///< 75 .. Optimization: Gather begin
  TpProcHtOptRegang,                  ///< 76 .. Optimization: Regang begin
  TpProcHtOptLinks,                   ///< 77 .. Optimization: Link Begin
  TpProcHtOptSubLinks,                ///< 78 .. Optimization: Sublinks begin
  TpProcHtOptFinish,                  ///< 79 .. Optimization: Set begin
  TpProcHtTrafficDist,                ///< 7A .. Traffic Distribution begin
  TpProcHtTuning,                     ///< 7B .. Misc Tuning Begin
  TpProcHtDone,                       ///< 7C .. HT Init complete
  TpProcHtApMapEntry,                 ///< 7D .. AP HT: Init Maps begin
  TpProcHtApMapDone,                  ///< 7E .. AP HT: Complete

  // Extended memory test point
  TpProcMemSendMRS2 = 0x80,           ///< 80 .. Sending MRS2
  TpProcMemSendMRS3,                  ///< 81 .. Sedding MRS3
  TpProcMemSendMRS1,                  ///< 82 .. Sending MRS1
  TpProcMemSendMRS0,                  ///< 83 .. Sending MRS0
  TpProcMemContinPatternGenRead,      ///< 84 .. Continuous Pattern Read
  TpProcMemContinPatternGenWrite,     ///< 85 .. Continuous Pattern Write
  TpProcMem__RdDqsTraining,           ///< 86 .. Mem: RdDqs Training begin
  TpProcMemBefore__TrainExtVrefChange,///< 87 .. Mem: Before optional callout to platfrom BIOS to change External Vref during training
  TpProcMemAfter__TrainExtVrefChange, ///< 88 .. Mem: After optional callout to platfrom BIOS to change External Vref during training

  StartNbTestPoints = 0x90,           ///< 90 Entry used for range testing for @b NorthBridge related TPs
  TpNbxxx,                            ///< 91 .
  EndNbTestPoints,                    ///< 92 End of TP range for NB

  StartFchTestPoints = 0xB0,          ///< B0 Entry used for range testing for @b FCH related TPs
  TpFchInitResetDispatching,          ///< B1 .. FCH InitReset dispatch point
  TpFchGppBeforePortTraining,         ///< B2 .. Before FCH GPP port training
  TpFchGppGen1PortPolling,            ///< B3 .. FCH GPP port polling with GEN1 speed
  TpFchGppGen2PortPolling,            ///< B4 .. FCH GPP port polling with GEN2 speed
  TpFchGppAfterPortTraining,          ///< B5 .. After FCH GPP port training
  TpFchInitEnvDispatching,            ///< B6 .. FCH InitEnv dispatch point
  TpFchInitMidDispatching,            ///< B7 .. FCH InitMid dispatch point
  TpFchInitLateDispatching,           ///< B8 .. FCH InitLate dispatch point
  TpFchGppHotPlugging,                ///< B9 .. FCH GPP hot plug event
  TpFchGppHotUnplugging,              ///< BA .. AFCH GPP hot unplug event
  TpFchInitS3EarlyDispatching,        ///< BB .. FCH InitS3Early dispatch point
  TpFchInitS3LateDispatching,         ///< BC .. FCH InitS3Late dispatch point
  EndFchTestPoints,                   ///< BF End of TP range for FCH

  // Interface test points
  TpIfAmdInitResetEntry = 0xC0,       ///< C0 .. Entry to AmdInitReset
  TpIfAmdInitResetExit,               ///< C1 .. Exiting from AmdInitReset
  TpIfAmdInitRecoveryEntry,           ///< C2 .. Entry to AmdInitRecovery
  TpIfAmdInitRecoveryExit,            ///< C3 .. Exiting from AmdInitRecovery
  TpIfAmdInitEarlyEntry,              ///< C4 .. Entry to AmdInitEarly
  TpIfAmdInitEarlyExit,               ///< C5 .. Exiting from AmdInitEarly
  TpIfAmdInitPostEntry,               ///< C6 .. Entry to AmdInitPost
  TpIfAmdInitPostExit,                ///< C7 .. Exiting from AmdInitPost
  TpIfAmdInitEnvEntry,                ///< C8 .. Entry to AmdInitEnv
  TpIfAmdInitEnvExit,                 ///< C9 .. Exiting from AmdInitEnv
  TpIfAmdInitMidEntry,                ///< CA .. Entry to AmdInitMid
  TpIfAmdInitMidExit,                 ///< CB .. Exiting from AmdInitMid
  TpIfAmdInitLateEntry,               ///< CC .. Entry to AmdInitLate
  TpIfAmdInitLateExit,                ///< CD .. Exiting from AmdInitLate
  TpIfAmdS3SaveEntry,                 ///< CE .. Entry to AmdS3Save
  TpIfAmdS3SaveExit,                  ///< CF .. Exiting from AmdS3Save
  TpIfAmdInitResumeEntry,             ///< D0 .. Entry to AmdInitResume
  TpIfAmdInitResumeExit,              ///< D1 .. Exiting from AmdInitResume
  TpIfAmdS3LateRestoreEntry,          ///< D2 .. Entry to AmdS3LateRestore
  TpIfAmdS3LateRestoreExit,           ///< D3 .. Exiting from AmdS3LateRestore
  TpIfAmdLateRunApTaskEntry,          ///< D4 .. Entry to AmdS3LateRestore
  TpIfAmdLateRunApTaskExit,           ///< D5 .. Exiting from AmdS3LateRestore
  TpIfAmdReadEventLogEntry,           ///< D6 .. Entry to AmdReadEventLog
  TpIfAmdReadEventLogExit,            ///< D7 .. Exiting from AmdReadEventLog
  TpIfAmdGetApicIdEntry,              ///< D8 .. Entry to AmdGetApicId
  TpIfAmdGetApicIdExit,               ///< D9 .. Exiting from AmdGetApicId
  TpIfAmdGetPciAddressEntry,          ///< DA .. Entry to AmdGetPciAddress
  TpIfAmdGetPciAddressExit,           ///< DB .. Exiting from AmdGetPciAddress
  TpIfAmdIdentifyCoreEntry,           ///< DC .. Entry to AmdIdentifyCore
  TpIfAmdIdentifyCoreExit,            ///< DD .. Exiting from AmdIdentifyCore
  TpIfBeforeRunApFromIds,             ///< DE .. After IDS calls out to run code on an AP
  TpIfAfterRunApFromIds,              ///< DF .. After IDS calls out to run code on an AP
  TpIfBeforeGetIdsData,               ///< E0 .. Before IDS calls out to get IDS data
  TpIfAfterGetIdsData,                ///< E1 .. After IDS calls out to get IDS data
  TpIfBeforeAllocateHeapBuffer,       ///< E2 .. Before the heap manager calls out to allocate a buffer
  TpIfAfterAllocateHeapBuffer,        ///< E3 .. After the heap manager calls out to allocate a buffer
  TpIfBeforeDeallocateHeapBuffer,     ///< E4 .. Before the heap manager calls out to deallocate a buffer
  TpIfAfterDeallocateHeapBuffer,      ///< E5 .. After the heap manager calls out to deallocate a buffer
  TpIfBeforeLocateHeapBuffer,         ///< E6 .. Before the heap manager calls out to locate a buffer
  TpIfAfterLocateHeapBuffer,          ///< E7 .. After the heap manager calls out to locate a buffer
  TpIfBeforeRunApFromAllAps,          ///< E8 .. Before the BSP calls out to run code on an AP
  TpIfAfterRunApFromAllAps,           ///< E9 .. After the BSP calls out to run code on an AP
  TpIfBeforeRunApFromAllCore0s,       ///< EA .. Before the BSP calls out to run code on an AP
  TpIfAfterRunApFromAllCore0s,        ///< EB .. After the BSP calls out to run code on an AP
  TpIfBeforeAllocateS3SaveBuffer,     ///< EC .. Before the S3 save code calls out to allocate a buffer
  TpIfAfterAllocateS3SaveBuffer,      ///< ED .. After the S3 save code calls out to allocate a buffer
  TpIfBeforeAllocateMemoryS3SaveBuffer,  ///< EE .. Before the memory S3 save code calls out to allocate a buffer
  TpIfAfterAllocateMemoryS3SaveBuffer,   ///< EF .. After the memory S3 save code calls out to allocate a buffer
  TpIfBeforeLocateS3PciBuffer,        ///< F0 .. Before the memory code calls out to locate a buffer
  TpIfAfterLocateS3PciBuffer,         ///< F1 .. After the memory code calls out to locate a buffer
  TpIfBeforeLocateS3CPciBuffer,       ///< F2 .. Before the memory code calls out to locate a buffer
  TpIfAfterLocateS3CPciBuffer,        ///< F3 .. After the memory code calls out to locate a buffer
  TpIfBeforeLocateS3MsrBuffer,        ///< F4 .. Before the memory code calls out to locate a buffer
  TpIfAfterLocateS3MsrBuffer,         ///< F5 .. After the memory code calls out to locate a buffer
  TpIfBeforeLocateS3CMsrBuffer,       ///< F6 .. Before the memory code calls out to locate a buffer
  TpIfAfterLocateS3CMsrBuffer,        ///< F7 .. After the memory code calls out to locate a buffer
  TpPerfUnit,                         ///< F8 .. The Unit of performance measure.
  EndAgesaTps = 0xFF,                 ///< Last defined AGESA TP
};
"""

cparser = cstruct.cstruct()
cparser.load(TPS)

TPS_SPLIT = {}

for l in TPS.split("\n"):
    l = l.strip()
    if l.startswith("enum") or l.startswith("}") or l.startswith("//") or not l:
        continue
    s = l.split(",", 1)
    if len(s) != 2:
        continue
    name = s[0]
    comment = s[1]
    if "=" in name:
        name = name.split("=")[0].strip()
    comment = comment.strip()
    # print(f"{name=} {comment=}")
    TPS_SPLIT[name] = comment
    
# print(cparser.AGESA_TP)
for t in cparser.AGESA_TP:
    comment = TPS_SPLIT[t.name]
    line = f"{t.value:#04x}, \"{t.name}\""
    line = "{" + line + "}," + (" " * (50 - len(line))) + comment
    print(line)
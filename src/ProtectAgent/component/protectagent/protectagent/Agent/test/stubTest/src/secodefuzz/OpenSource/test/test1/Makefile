#$(warning  $(XXX))


#编译secodefuzz库
CFLAGS_secodefuzz  	= -Wall -g -fPIC -fno-omit-frame-pointer -O0
CC      			= /usr/bin/gcc 

LIB     			= ar -ru   ## ar -crv
SHELL   			= sh
RM      			= rm -rf
MKDIR   			= mkdir -p
export C_INCLUDE_PATH=$C_INCLUDE_PATH:../../../examples/xml-lib/libxml2-2.6.26/include

#clang仅仅.a库才支持覆盖反馈
all:Secodepits_so Secodepits_so_ex Secodefuzz_so Secodefuzz_a

Secodepits_so:
	@$(CC) $(CFLAGS_secodefuzz) \
	../../../Secodepits/xml/Xlib.c \
	../../../Secodepits/xml/XParseHW.c \
	../../../Secodepits/xml/XParseXml.c \
	../../../Secodepits/xml/xparsexml/XParseXml_Config.c \
	../../../Secodepits/xml/xparsexml/XParseXml_TestModel.c \
	../../../Secodepits/xml/xparsexml/XParseXml_StateModel.c \
	../../../Secodepits/xml/xparsexml/XParseXml_DataModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_DataModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_StateModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_TestModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_Associated.c \
	../../../Secodepits/xml/xparsepits/XParsePits_Bin.c \
	../../../Secodepits/xml/XCommon.c \
	../../../Secodepits/xml/XDebug.c \
	../../../Secodepits/xml/XFileIO.c \
	../../../Secodepits/xml/xdo/XDoPadding.c \
	../../../Secodepits/xml/xdo/XDoRelation.c \
	../../../Secodepits/xml/xdo/XDoFixup.c \
	../../../Secodepits/xml/xdo/XDoAnalyzer.c \
	../../../Secodepits/xml/xdo/XDoConstraint.c \
	../../../Secodepits/xml/xdo/XDoTransformer.c \
	../../../Secodepits/xml/xdo/XDoPublisher.c \
	../../../Secodepits/xml/xdo/XDoAction.c \
	../../../Secodepits/xml/xdo/XDoState.c \
	../../../Secodepits/xml/xself/xfixup/XFixupIcmpChecksum.c \
	../../../Secodepits/xml/xself/xfixup/XFixupUDPChecksum.c \
	../../../Secodepits/xml/xself/xfixup/XFixupcrc32.c \
	../../../Secodepits/xml/xself/xanalyzer/XAnalyaerXml.c \
	../../../Secodepits/xml/xself/xanalyzer/XAnalyaerJson.c \
	../../../Secodepits/xml/xself/xanalyzer/cJSON.c \
	../../../Secodepits/xml/xself/xexpression/XExpression.c \
	../../../Secodepits/xml/xself/xconstraint/XConstraint.c \
	../../../Secodepits/xml/xself/xtransformer/XTransformerInserta.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_raw.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_udp.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_tcp.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_debug.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_printf.c \
    ../../../Secodepits/xml/xself/xpublisher/XPublisher_buf.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_file.c \
	../../../Secodepits/xml/xmutators/XMutatorElement.c \
	../../../Secodepits/xml/xmutators/XMutatorPits.c \
	../../../Secodepits/xml/XExternal.c \
	-shared -o  libSecodepits.so
	@echo "*******************creat libSecodepits.so success"

Secodepits_so_ex:
	@$(CC) $(CFLAGS_secodefuzz) -D USE_libxm2lib -D Open_source \
	../../../Secodepits/xml/Xlib.c \
	../../../Secodepits/xml/XParseHW.c \
	../../../Secodepits/xml/XParseXml.c \
	../../../Secodepits/xml/xparsexml/XParseXml_Config.c \
	../../../Secodepits/xml/xparsexml/XParseXml_TestModel.c \
	../../../Secodepits/xml/xparsexml/XParseXml_StateModel.c \
	../../../Secodepits/xml/xparsexml/XParseXml_DataModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_DataModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_StateModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_TestModel.c \
	../../../Secodepits/xml/xparsepits/XParsePits_Associated.c \
	../../../Secodepits/xml/xparsepits/XParsePits_Bin.c \
	../../../Secodepits/xml/XCommon.c \
	../../../Secodepits/xml/XDebug.c \
	../../../Secodepits/xml/XFileIO.c \
	../../../Secodepits/xml/xdo/XDoPadding.c \
	../../../Secodepits/xml/xdo/XDoRelation.c \
	../../../Secodepits/xml/xdo/XDoFixup.c \
	../../../Secodepits/xml/xdo/XDoAnalyzer.c \
	../../../Secodepits/xml/xdo/XDoConstraint.c \
	../../../Secodepits/xml/xdo/XDoTransformer.c \
	../../../Secodepits/xml/xdo/XDoPublisher.c \
	../../../Secodepits/xml/xdo/XDoAction.c \
	../../../Secodepits/xml/xdo/XDoState.c \
	../../../Secodepits/xml/xself/xfixup/XFixupIcmpChecksum.c \
	../../../Secodepits/xml/xself/xfixup/XFixupUDPChecksum.c \
	../../../Secodepits/xml/xself/xfixup/XFixupcrc32.c \
	../../../Secodepits/xml/xself/xanalyzer/XAnalyaerXml.c \
	../../../Secodepits/xml/xself/xanalyzer/XAnalyaerJson.c \
	../../../Secodepits/xml/xself/xanalyzer/cJSON.c \
	../../../Secodepits/xml/xself/xexpression/XExpression.c \
	../../../Secodepits/xml/xself/xconstraint/XConstraint.c \
	../../../Secodepits/xml/xself/xtransformer/XTransformerInserta.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_raw.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_udp.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_tcp.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_debug.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_printf.c \
    ../../../Secodepits/xml/xself/xpublisher/XPublisher_buf.c \
	../../../Secodepits/xml/xself/xpublisher/XPublisher_file.c \
	../../../Secodepits/xml/xmutators/XMutatorElement.c \
	../../../Secodepits/xml/xmutators/XMutatorPits.c \
	../../../Secodepits/xml/XExternal.c \
	-shared -o  libSecodepits.so
	@echo "*******************creat libSecodepits.so success"
	
Secodefuzz_so:
	@$(CC) $(CFLAGS_secodefuzz) \
	../../../Secodefuzz/common/PCommon.c \
	../../../Secodefuzz/common/PCorpus.c \
	../../../Secodefuzz/common/PCorpusIO.c \
	../../../Secodefuzz/common/PCorpusSchedule.c \
	../../../Secodefuzz/common/PDebug.c \
	../../../Secodefuzz/common/PExternal.c \
	../../../Secodefuzz/common/PHexDump.c \
	../../../Secodefuzz/common/PInternal.c \
	../../../Secodefuzz/common/PIO.c \
	../../../Secodefuzz/common/PKcov.c \
	../../../Secodefuzz/common/PLLVM8BitCounters.c \
	../../../Secodefuzz/common/PLLVMData.c \
	../../../Secodefuzz/common/PLLVMHook.c \
	../../../Secodefuzz/common/PLLVMLEAKCHECK.c \
	../../../Secodefuzz/common/PLLVMPcCounters.c \
	../../../Secodefuzz/common/PLLVMTracePC.c \
	../../../Secodefuzz/common/PMother.c \
	../../../Secodefuzz/common/PPublic.c \
	../../../Secodefuzz/common/PRandom.c \
	../../../Secodefuzz/common/PReport.c \
	../../../Secodefuzz/common/PSHA1.c \
	../../../Secodefuzz/common/PSignalCallback.c \
	../../../Secodefuzz/common/PTimeOut.c \
    ../../../Secodefuzz/common/PCallBack.c \
	../../../Secodefuzz/mutators/Dictionary.c \
	../../../Secodefuzz/mutators/MBlobChangeBinaryInteger.c \
	../../../Secodefuzz/mutators/MBlobChangeFromNull.c \
	../../../Secodefuzz/mutators/MBlobChangeRandom.c \
	../../../Secodefuzz/mutators/MBlobChangeSpecial.c \
	../../../Secodefuzz/mutators/MBlobChangeToNull.c \
	../../../Secodefuzz/mutators/MBlobEnum.c \
	../../../Secodefuzz/mutators/MBlobExpandAllRandom.c \
	../../../Secodefuzz/mutators/MBlobExpandSingleIncrementing.c \
	../../../Secodefuzz/mutators/MBlobExpandSingleRandom.c \
	../../../Secodefuzz/mutators/MBlobExpandZero.c \
	../../../Secodefuzz/mutators/MBlobMagic.c \
	../../../Secodefuzz/mutators/Mcommon.c \
	../../../Secodefuzz/mutators/MCustomBlob.c \
	../../../Secodefuzz/mutators/MCustomNumber.c \
	../../../Secodefuzz/mutators/MCustomString.c \
	../../../Secodefuzz/mutators/MDataElementAFL.c \
	../../../Secodefuzz/mutators/MDataElementBitFill.c \
	../../../Secodefuzz/mutators/MDataElementBitFlipper.c \
	../../../Secodefuzz/mutators/MDataElementBitZero.c \
	../../../Secodefuzz/mutators/MDataElementByteRandom.c \
	../../../Secodefuzz/mutators/MDataElementChangeASCIIInteger.c \
	../../../Secodefuzz/mutators/MDataElementCopyPartOf.c \
	../../../Secodefuzz/mutators/MDataElementDuplicate.c \
	../../../Secodefuzz/mutators/MDataElementInsertPartOf.c \
	../../../Secodefuzz/mutators/MDataElementLengthEdgeCase.c \
	../../../Secodefuzz/mutators/MDataElementLengthGauss.c \
	../../../Secodefuzz/mutators/MDataElementLengthRandom.c \
	../../../Secodefuzz/mutators/MDataElementLengthRepeatPart.c \
	../../../Secodefuzz/mutators/MDataElementMagic.c \
	../../../Secodefuzz/mutators/MDataElementMBitFlipper.c \
	../../../Secodefuzz/mutators/MDataElementOneByteInsert.c \
	../../../Secodefuzz/mutators/MDataElementReduce.c \
	../../../Secodefuzz/mutators/MDataElementStringStatic.c \
	../../../Secodefuzz/mutators/MDataElementSwapTwoPart.c \
	../../../Secodefuzz/mutators/MDataElementMagicChange.c \
	../../../Secodefuzz/mutators/MDouble.c \
	../../../Secodefuzz/mutators/MFloat16.c \
	../../../Secodefuzz/mutators/MFloat32.c \
	../../../Secodefuzz/mutators/MFloat64.c \
	../../../Secodefuzz/mutators/MIpv4.c \
	../../../Secodefuzz/mutators/MIpv6.c \
	../../../Secodefuzz/mutators/MMac.c \
	../../../Secodefuzz/mutators/MNumberEdgeCase.c \
	../../../Secodefuzz/mutators/MNumberEdgeRange.c \
	../../../Secodefuzz/mutators/MNumberEnum.c \
	../../../Secodefuzz/mutators/MNumberMagic.c \
	../../../Secodefuzz/mutators/MNumberPowerRandom.c \
	../../../Secodefuzz/mutators/MNumberRandom.c \
	../../../Secodefuzz/mutators/MNumberRange.c \
	../../../Secodefuzz/mutators/MNumberSmallRange.c \
	../../../Secodefuzz/mutators/MNumberVariance.c \
	../../../Secodefuzz/mutators/MSelf.c \
	../../../Secodefuzz/mutators/MCustomMutator.c \
	../../../Secodefuzz/mutators/MCross.c \
	../../../Secodefuzz/mutators/MStringAsciiRandom.c \
	../../../Secodefuzz/mutators/MStringCaseLower.c \
	../../../Secodefuzz/mutators/MStringCaseRandom.c \
	../../../Secodefuzz/mutators/MStringCaseUpper.c \
	../../../Secodefuzz/mutators/MStringEnum.c \
	../../../Secodefuzz/mutators/MStringLengthAsciiRandom.c \
	../../../Secodefuzz/mutators/MStringLengthEdgeCase.c \
	../../../Secodefuzz/mutators/MStringLengthGauss.c \
	../../../Secodefuzz/mutators/MStringLengthRandom.c \
	../../../Secodefuzz/mutators/MStringMagic.c \
	../../../Secodefuzz/mutators/MStringStatic.c \
	../../../Secodefuzz/mutators/MStringUtf8Bom.c \
	../../../Secodefuzz/mutators/MStringUtf8BomLength.c \
	../../../Secodefuzz/mutators/MStringUtf8BomStatic.c \
	-shared -o libSecodefuzz.so 
	@echo "*******************creat libSecodefuzz.so success"
	
Secodefuzz_a:
	@$(CC) $(CFLAGS_secodefuzz) -c \
	../../../Secodefuzz/common/PCommon.c \
	../../../Secodefuzz/common/PCorpus.c \
	../../../Secodefuzz/common/PCorpusIO.c \
	../../../Secodefuzz/common/PCorpusSchedule.c \
	../../../Secodefuzz/common/PDebug.c \
	../../../Secodefuzz/common/PExternal.c \
	../../../Secodefuzz/common/PHexDump.c \
	../../../Secodefuzz/common/PInternal.c \
	../../../Secodefuzz/common/PIO.c \
	../../../Secodefuzz/common/PKcov.c \
	../../../Secodefuzz/common/PLLVM8BitCounters.c \
	../../../Secodefuzz/common/PLLVMData.c \
	../../../Secodefuzz/common/PLLVMHook.c \
	../../../Secodefuzz/common/PLLVMLEAKCHECK.c \
	../../../Secodefuzz/common/PLLVMPcCounters.c \
	../../../Secodefuzz/common/PLLVMTracePC.c \
	../../../Secodefuzz/common/PMother.c \
	../../../Secodefuzz/common/PPublic.c \
	../../../Secodefuzz/common/PRandom.c \
	../../../Secodefuzz/common/PReport.c \
	../../../Secodefuzz/common/PSHA1.c \
	../../../Secodefuzz/common/PSignalCallback.c \
	../../../Secodefuzz/common/PTimeOut.c \
    ../../../Secodefuzz/common/PCallBack.c \
	../../../Secodefuzz/mutators/Dictionary.c \
	../../../Secodefuzz/mutators/MBlobChangeBinaryInteger.c \
	../../../Secodefuzz/mutators/MBlobChangeFromNull.c \
	../../../Secodefuzz/mutators/MBlobChangeRandom.c \
	../../../Secodefuzz/mutators/MBlobChangeSpecial.c \
	../../../Secodefuzz/mutators/MBlobChangeToNull.c \
	../../../Secodefuzz/mutators/MBlobEnum.c \
	../../../Secodefuzz/mutators/MBlobExpandAllRandom.c \
	../../../Secodefuzz/mutators/MBlobExpandSingleIncrementing.c \
	../../../Secodefuzz/mutators/MBlobExpandSingleRandom.c \
	../../../Secodefuzz/mutators/MBlobExpandZero.c \
	../../../Secodefuzz/mutators/MBlobMagic.c \
	../../../Secodefuzz/mutators/Mcommon.c \
	../../../Secodefuzz/mutators/MCustomBlob.c \
	../../../Secodefuzz/mutators/MCustomNumber.c \
	../../../Secodefuzz/mutators/MCustomString.c \
	../../../Secodefuzz/mutators/MDataElementAFL.c \
	../../../Secodefuzz/mutators/MDataElementBitFill.c \
	../../../Secodefuzz/mutators/MDataElementBitFlipper.c \
	../../../Secodefuzz/mutators/MDataElementBitZero.c \
	../../../Secodefuzz/mutators/MDataElementByteRandom.c \
	../../../Secodefuzz/mutators/MDataElementChangeASCIIInteger.c \
	../../../Secodefuzz/mutators/MDataElementCopyPartOf.c \
	../../../Secodefuzz/mutators/MDataElementDuplicate.c \
	../../../Secodefuzz/mutators/MDataElementInsertPartOf.c \
	../../../Secodefuzz/mutators/MDataElementLengthEdgeCase.c \
	../../../Secodefuzz/mutators/MDataElementLengthGauss.c \
	../../../Secodefuzz/mutators/MDataElementLengthRandom.c \
	../../../Secodefuzz/mutators/MDataElementLengthRepeatPart.c \
	../../../Secodefuzz/mutators/MDataElementMagic.c \
	../../../Secodefuzz/mutators/MDataElementMBitFlipper.c \
	../../../Secodefuzz/mutators/MDataElementOneByteInsert.c \
	../../../Secodefuzz/mutators/MDataElementReduce.c \
	../../../Secodefuzz/mutators/MDataElementStringStatic.c \
	../../../Secodefuzz/mutators/MDataElementSwapTwoPart.c \
	../../../Secodefuzz/mutators/MDataElementMagicChange.c \
	../../../Secodefuzz/mutators/MDouble.c \
	../../../Secodefuzz/mutators/MFloat16.c \
	../../../Secodefuzz/mutators/MFloat32.c \
	../../../Secodefuzz/mutators/MFloat64.c \
	../../../Secodefuzz/mutators/MIpv4.c \
	../../../Secodefuzz/mutators/MIpv6.c \
	../../../Secodefuzz/mutators/MMac.c \
	../../../Secodefuzz/mutators/MNumberEdgeCase.c \
	../../../Secodefuzz/mutators/MNumberEdgeRange.c \
	../../../Secodefuzz/mutators/MNumberEnum.c \
	../../../Secodefuzz/mutators/MNumberMagic.c \
	../../../Secodefuzz/mutators/MNumberPowerRandom.c \
	../../../Secodefuzz/mutators/MNumberRandom.c \
	../../../Secodefuzz/mutators/MNumberRange.c \
	../../../Secodefuzz/mutators/MNumberSmallRange.c \
	../../../Secodefuzz/mutators/MNumberVariance.c \
	../../../Secodefuzz/mutators/MSelf.c \
	../../../Secodefuzz/mutators/MCustomMutator.c \
	../../../Secodefuzz/mutators/MCross.c \
	../../../Secodefuzz/mutators/MStringAsciiRandom.c \
	../../../Secodefuzz/mutators/MStringCaseLower.c \
	../../../Secodefuzz/mutators/MStringCaseRandom.c \
	../../../Secodefuzz/mutators/MStringCaseUpper.c \
	../../../Secodefuzz/mutators/MStringEnum.c \
	../../../Secodefuzz/mutators/MStringLengthAsciiRandom.c \
	../../../Secodefuzz/mutators/MStringLengthEdgeCase.c \
	../../../Secodefuzz/mutators/MStringLengthGauss.c \
	../../../Secodefuzz/mutators/MStringLengthRandom.c \
	../../../Secodefuzz/mutators/MStringMagic.c \
	../../../Secodefuzz/mutators/MStringStatic.c \
	../../../Secodefuzz/mutators/MStringUtf8Bom.c \
	../../../Secodefuzz/mutators/MStringUtf8BomLength.c \
	../../../Secodefuzz/mutators/MStringUtf8BomStatic.c 
	@ar cr libSecodefuzz.a *.o
	@$(RM) *.o
	@echo "*******************creat libSecodefuzz.a success"
	
########################################
clean:
	@$(RM) *.o
	@$(RM) *.a
	@$(RM) *.so


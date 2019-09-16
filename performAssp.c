#include "emscripten.h"
#include "assp/asspdsp.h" // sic remove this!

#include "wasmassp.h"
#include <asspana.h>
#include <dataobj.h>
#include <asspfio.h>
#include <asspmess.h>
#include <spectra.h>
#include <asspdsp.h>
#include <assp.h>
#include <stdlib.h>
#include <ipds_lbl.h>
#include <diff.h>               /* shouldn't asspana include diff.h? */
#include <filter.h>
#include <ksv.h>
#include <ctype.h>              /* tolower() */
#include <string.h>
#include <stdio.h>

/*
 * This list is used to map gender option values from R to the appropriate 
 * value for assp 
 */
W_GENDER        gender[] = {
    {"female", TG_FEMALE, 'f'}
    ,
    /*
     * {"FEMALE", TG_FEMALE, 'f'} 
     */
    /*
     * ,
     */
    {"male", TG_MALE, 'm'}
    ,
    /*
     * {"MALE", TG_MALE, 'm'} 
     */
    /*
     * ,
     */
    {"unknown", TG_UNKNOWN, 'u'}
    ,
    /*
     * {"UNKNOWN", TG_UNKNOWN, 'u'} 
     */
    /*
     * ,
     */
    {NULL, TG_NONE, '0'}
};
/*
 * OPTION LISTS: Each analysis function uses only a subset of all possible 
 * assp option. A list of legal/supported options is provided here for
 * each of the analysis functions in wrassp. 
 */

/*
 * ACFANA option list
 */
W_OPT           acfanaOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"effectiveLength", WO_MSEFFLEN}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"window", WO_WINFUNC}
    ,
    {"analysisOrder", WO_ORDER}
    ,
    {"energyNormalization", WO_ENERGYNORM}
    ,
    {"lengthNormalization", WO_LENGTHNORM}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {"progressBar", WO_PBAR}
    ,
    {NULL, WO_NONE}
};

/*
 * AFDIFF option list 
 */
W_OPT           afdiffOptions[] = {
    {"computeBackwardDifference", WO_DIFF_OPT_BACKWARD}
    ,
    {"computeCentralDifference", WO_DIFF_OPT_CENTRAL}
    ,
    {"channel", WO_CHANNEL}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {"progressBar", WO_PBAR}
    ,
    {NULL, WO_NONE}
};

/*
 * AFFILTER option list 
 */
W_OPT           affilterOptions[] = {
    {"highPass", WO_HPCUTOFF}
    ,
    {"lowPass", WO_LPCUTOFF}
    ,
    {"stopBand", WO_STOPDB}
    ,
    {"transition", WO_TBWIDTH}
    ,
    {"useIIR", WO_USEIIR}
    ,
    {"numIIRsections", WO_NUMIIRSECS}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    /*
     * {"-channel"}
     */
    /*
     * , 
     */
    /*
     * {"-noDither"}
     */
    /*
     * , 
     */
    /*
     * {"-gain"}
     */
    /*
     * , 
     */
    {NULL, WO_NONE}
};

/*
 * F0_KSV (aka ksvF0, f0ana) option list 
 */
W_OPT           f0_ksvOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"gender", WO_GENDER}
    ,
    {"maxF", WO_MAXF}
    ,
    {"minF", WO_MINF}
    ,
    {"minAmp", WO_VOIMAG}
    ,
    {"maxZCR", WO_VOIZCR}
    ,
    {"progressBar", WO_PBAR}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * MHSF0 (aka f0_mhs, mhspitch) option list 
 */
W_OPT           f0_mhsOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"gender", WO_GENDER}
    ,
    {"maxF", WO_MAXF}
    ,
    {"minF", WO_MINF}
    ,
    {"minAmp", WO_VOIMAG}
    ,
    {"minAC1", WO_VOIAC1PP}
    ,
    {"minRMS", WO_VOIRMS}
    ,
    {"maxZCR", WO_VOIZCR}
    ,
    {"minProb", WO_VOIPROB}
    ,
    {"progressBar", WO_PBAR}
    ,
    {"plainSpectrum", WO_MHS_OPT_POWER}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * FOREST option list
 */
W_OPT           forestOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"effectiveLength", WO_MSEFFLEN}
    ,
    {"nominalF1", WO_NOMF1}
    ,
    {"gender", WO_GENDER}
    ,
    {"estimate", WO_INS_EST}
    ,
    {"order", WO_ORDER}
    ,
    {"incrOrder", WO_INCREMENT}
    ,
    {"numFormants", WO_NUMFORMANTS}
    ,
    {"window", WO_WINFUNC}
    ,
    {"preemphasis", WO_PREEMPH}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * RFCANA option list 
 */
W_OPT           rfcanaOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"effectiveLength", WO_MSEFFLEN}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"window", WO_WINFUNC}
    ,
    {"order", WO_ORDER}
    ,
    {"preemphasis", WO_PREEMPH}
    ,
    {"lpType", WO_TYPE}
    ,
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * RMSANA OPTION LIST
 */
W_OPT           rmsanaOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"effectiveLength", WO_MSEFFLEN}
    ,
    {"linear", WO_RMS_OPT_LINEAR}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"window", WO_WINFUNC}
    ,
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * SPECTRUM (implemented as dftSpectrum, cssSpectrum, lpsSpectrum in R)
 * option list 
 */
W_OPT           spectrumOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"spectrumType", WO_TYPE}
    ,
    {"resolution", WO_RESOLUTION}
    ,
    {"fftLength", WO_FFTLEN}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"window", WO_WINFUNC}
    ,

    /*
     * DFT spectrum 
     */
    {"bandwidth", WO_BANDWIDTH}
    ,

    /*
     * LP smoothed spectrum 
     */
    {"effectiveLength", WO_MSEFFLEN}
    ,
    {"order", WO_ORDER}
    ,
    {"preemphasis", WO_PREEMPH}
    ,
    {"deemphasize", WO_LPS_OPT_DEEMPH}
    ,

    /*
     * Cepstrally smoothed spectrum 
     */
    {"numCeps", WO_ORDER}
    ,

    /*
     * general stuff 
     */
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * ZCRANA option list
 */
W_OPT           zcranaOptions[] = {
    {"beginTime", WO_BEGINTIME}
    ,
    {"centerTime", WO_CENTRETIME}
    ,
    {"endTime", WO_ENDTIME}
    ,
    {"windowShift", WO_MSSHIFT}
    ,
    {"windowSize", WO_MSSIZE}
    ,
    {"explicitExt", WO_OUTPUTEXT}
    ,                           /* DON'T FORGET EXTENSION!!! */
    /*
     * {"Window", WO_WINFUNC} 
     */
    /*
     * , 
     */
    {"progressBar", WO_PBAR}
    ,
    {"toFile", WO_TOFILE}
    ,
    {"outputDirectory", WO_OUTPUTDIR}
    ,
    {NULL, WO_NONE}
};

/*
 * The following is a list of function descriptors: The descriptor
 * includes for each analysis function the name, a pointer to a function
 * that generates default setting, a pointer to a function that performs
 * the analysis a list of the legal/supported options, major and minor
 * assp version of the analysis routine, a default suffix and an
 * enumerator 
 */
A_F_LIST        funclist[] = {
    {"acfana", setACFdefaults, computeACF, acfanaOptions, ACF_MAJOR,
     ACF_MINOR, ACF_DEF_SUFFIX, AF_ACFANA}
    ,
    {"afdiff", setDiffDefaults, diffSignal, afdiffOptions, DIFF_MAJOR,
     DIFF_MINOR, "AUTO", AF_AFDIFF}
    ,
    {"affilter", setFILTdefaults, computeFilter, affilterOptions,
     FILT_MAJOR, FILT_MINOR,
     "", AF_AFFILTER}
    ,
    {"f0ana", setKSVdefaults, computeF0, f0_ksvOptions, KSV_MAJOR,
     KSV_MINOR,
     KSV_DEF_SUFFIX, AF_KSV_PITCH}
    ,
    {"forest", setFMTdefaults, computeFMT, forestOptions, FMT_MAJOR,
     FMT_MINOR,
     FMT_DEF_SUFFIX, AF_FOREST}
    ,
    {"mhspitch", setMHSdefaults, computeMHS, f0_mhsOptions, MHS_MAJOR,
     MHS_MINOR,
     MHS_DEF_SUFFIX, AF_MHS_PITCH}
    ,
    {"rfcana", setLPdefaults, computeLP, rfcanaOptions, RFC_MAJOR,
     RFC_MINOR,
     "", AF_RFCANA}
    ,
    {"rmsana", setRMSdefaults, computeRMS, rmsanaOptions, RMS_MAJOR,
     RMS_MINOR,
     RMS_DEF_SUFFIX, AF_RMSANA}
    ,
    {"spectrum", setSPECTdefaults, computeSPECT, spectrumOptions,
     SPECT_MAJOR, SPECT_MINOR,
     "", AF_SPECTRUM}
    ,
    {"zcrana", setZCRdefaults, computeZCR, zcranaOptions, ZCR_MAJOR,
     ZCR_MINOR,
     ZCR_DEF_SUFFIX, AF_ZCRANA}
    ,
    {NULL, NULL, NULL, 0, 0, AF_NONE}
};

/* write DOBJ to a tab sep. values file
 * code taken from dobj2AsspDataObj(DOBJ * data)
 * & getDObjTrackData()
 */ 
int dobj2tsv(DOBJ * data, FILE *f){

    DDESC          *desc = NULL;
    TSSFF_Generic  *genVar = NULL;
    int             i,
                    m,
                    n;

    /*
     * print column names
     */
    fprintf(f, "ftime\t");
    for (i = 0, desc = &(data->ddl); desc != NULL; desc = desc->next, i++) {
      for (n = 0; n < desc->numFields; n++) {
        fprintf(f, "%s_%i(%s)", desc->ident, n + 1, desc->unit);
        if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
      }
     }
    fprintf(f, "\n"); // done with row

    /*
     * print every record
     */

    // buffer to hold current record
    void           *tempBuffer,
                    *bufPtr;

    tempBuffer = malloc((size_t) data->recordSize);

    /*
     * various pointers for variuos data sizes
     */
      uint8_t        *u8Ptr;
      int8_t         *i8Ptr;
      uint16_t       *u16Ptr;
      int16_t        *i16Ptr;
      uint32_t       *u32Ptr;
      int32_t        *i32Ptr;
      float          *f32Ptr;
      double         *f64Ptr;

      double         *Rans;
      int            *Ians;
      uint8_t        *bPtr;
      bPtr = (uint8_t *) tempBuffer;
      i = 0;                      /* initial index in buffer */


      for (m = 0; m < data->bufNumRecs; m++) {
          // print frame time column
          fprintf(f, "%f\t", data->Start_Time + m * 1/data->dataRate);
          bufPtr = (void *)((char *)data->dataBuffer + m * data->recordSize);
          memcpy(tempBuffer, bufPtr, (size_t) data->recordSize);
          for (i = 0, desc = &(data->ddl); desc != NULL; desc = desc->next, i++) {
          switch (desc->format) {
          case DF_UINT8:
              {
                  u8Ptr = &bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%i", (unsigned int) u8Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_INT8:
              {
                  i8Ptr = (int8_t *) & bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%i", (int) u8Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_UINT16:
              {
                  u16Ptr = (uint16_t *) & bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                    fprintf(f, "%i", (unsigned int) u16Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_INT16:
              {
                  i16Ptr = (int16_t *) & bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%i", (int) i16Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_UINT32:
              {
                  u32Ptr = (uint32_t *) & bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%lu", (unsigned long) u32Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_INT32:
              {
                  i32Ptr = (int32_t *) & bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%li", (long) i32Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_REAL32:
              {
                  f32Ptr = (float *) &bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                        fprintf(f, "%f", (double) f32Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          case DF_REAL64:
              {
                  f64Ptr = (double *) &bPtr[desc->offset];
                  for (n = 0; n < desc->numFields; n++) {
                      fprintf(f, "%f", (double) f64Ptr[n]);
                      if(!(desc->next == NULL && n == desc->numFields - 1)) fprintf(f, "\t");
                  }
              }
              break;
          default:
              fprintf(stderr, "Hi, I just landed in the default of a switch in dataobj.c."
                  "I am sorry, I should not be here and I don't know what to do.");
              break;
          } // switch
          }
          fprintf(f, "\n"); // end of row
      } // for
      // clean up
      free(tempBuffer);

    return 0;
}


/*
 * This function performs an ASSP analysis routine 
 * @param audio_in pointer to an uint8_t buffer containing the audio samples
 * @param function_id int specifying which function to apply (currently only the 
 * default options are available)
 */
EMSCRIPTEN_KEEPALIVE
int performAssp(const char* audio_path, 
                const char* out_path, 
                const char* function_name,
                const char* file_type) {

  AOPTS           OPTS;
  AOPTS          *opt = &OPTS;
  A_F_LIST       *anaFunc = funclist;

  DOBJ           *inPtr,
                *outPtr;

  /*
    * Third parameter must be ASSP function name
    * check for validity and pick the right function descriptor 
    */
  while (anaFunc->funcNum != AF_NONE) {
      if (strcmp(function_name, anaFunc->fName) == 0){
        break;
      }
      anaFunc++;
  }
  if (anaFunc->funcNum == AF_NONE){
      fprintf(stderr, "Invalid analysis function in performAssp.c: %s\n", function_name);
      return 1;
  }

  /*
    * generate the default settings for the analysis function
    */
  if ((anaFunc->setFunc) (opt) == -1){
      fprintf(stderr, "%d\t$%s\n", asspMsgNum, getAsspMsg(asspMsgNum));
      return 1;
  }
  
  /*
    * open
    */
  inPtr = asspFOpen(strdup(audio_path), AFO_READ, (DOBJ *) NULL);
  if (inPtr == NULL){
      fprintf(stderr, "%s (%s)\n", getAsspMsg(asspMsgNum), strdup(audio_path));
  }

  /*
    * run the function (as pointed to in the descriptor) to generate
    * the output object 
    */
  outPtr = (anaFunc->compProc) (inPtr, opt, (DOBJ *) NULL);
  if (outPtr == NULL) {
      asspFClose(inPtr, AFC_FREE);
      fprintf(stderr, "%s (%s)\n", getAsspMsg(asspMsgNum), strdup(audio_path));
  }

  /*
   * input data object no longer needed 
   */
  asspFClose(inPtr, AFC_FREE);

  /*
   * write to file (either TSV (tab sep. values) or SSFF)
   * depending on file_type arg
   */
  if(strcmp(file_type, "TSV") == 0){
    FILE *f = fopen(out_path, "w");
    if (f == NULL){
        fprintf(stderr, "Error opening out_path file!\n");
        exit(1);
    }
    int res = dobj2tsv(outPtr, f);
    fclose(f);
  } else {

    outPtr = asspFOpen(out_path, AFO_WRITE, outPtr);
    if (outPtr == NULL) {
        asspFClose(outPtr, AFC_FREE);
        printf("%s (%s)\n", getAsspMsg(asspMsgNum), strdup(out_path));
    }
    asspFClose(outPtr, AFC_FREE);
  }
  return 0;
}


/*
 * Wrapper functions for filtering and for ksv f0 analysis 
 *
 * all other analyses come with a 'computeXYZ' functions with identical 
 * signatures but for various reasons these two do not. These function provide
 * wrappers.
 */
DOBJ           *
computeFilter(DOBJ * inpDOp, AOPTS * anaOpts, DOBJ * outDOp)
{
    int             FILE_IN,
                    FILE_OUT,
                    CREATED;
    DOBJ           *filtDOp = NULL;
    if (inpDOp == NULL || anaOpts == NULL || outDOp != NULL) {
        setAsspMsg(AEB_BAD_ARGS, "computeFilter");
        return (NULL);
    }
    FILE_IN = FILE_OUT = CREATED = FALSE;
    /*
     * check input object 
     */
    if (inpDOp->fp != NULL) {
        if (inpDOp->numRecords <= 0) {
            setAsspMsg(AEF_EMPTY, inpDOp->filePath);
            return (NULL);
        }
        FILE_IN = TRUE;
    } else if (anaOpts == NULL ||
               (anaOpts != NULL && !(anaOpts->options & AOPT_INIT_ONLY))) {
        if (inpDOp->dataBuffer == NULL || inpDOp->bufNumRecs <= 0) {
            setAsspMsg(AED_NO_DATA, "(computeFilter)");
            return (NULL);
        }
    }


    filtDOp = createFilter(inpDOp, anaOpts);
    if (filtDOp == NULL) {
        return (NULL);
    }

    if ((outDOp = filterSignal(inpDOp, filtDOp, outDOp)) == NULL) {
        return (NULL);
    }
    filtDOp = destroyFilter(filtDOp);
    return (outDOp);
}

DOBJ           *
computeF0(DOBJ * inpDOp, AOPTS * anaOpts, DOBJ * outDOp)
{
    return computeKSV(inpDOp, anaOpts, outDOp, (DOBJ *) NULL);
}
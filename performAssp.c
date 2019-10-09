#include "emscripten.h"
#include "assp/asspdsp.h" // sic remove this!

#include "wasmassp.h"

#include <cjson/cJSON.h>

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
#include <stdbool.h>

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
                const char* params_json,
                const char* file_type) {

    AOPTS           OPTS;
    AOPTS          *opt = &OPTS;
    W_OPT          *wrasspOptions;
    A_F_LIST       *anaFunc = funclist;
    int             tmp,
                    expExt = 0;
    char            ext[SUFF_MAX + 1] = "";
    W_GENDER       *gend = NULL;
    WFLIST         *wPtr = NULL;
    LP_TYPE        *lPtr = NULL;
    SPECT_TYPE     *sPtr = NULL;

    DOBJ           *inPtr,
                   *outPtr;

    int status = 0;

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
  * set options by parsing JSON option string
  */
    cJSON *json = cJSON_Parse(params_json);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }
    
    // get first element
    cJSON *cjsonp = json->child;
    char *name;
    while(cjsonp){
        name = cjsonp->string;
        wrasspOptions = anaFunc->options;
        while (wrasspOptions->name != NULL) {
            if (strcmp(wrasspOptions->name, name) == 0)
                break;
            wrasspOptions++;
        }
        if (wrasspOptions->name == NULL){
            fprintf(stderr, "Invalid option %s for ASSP analysis %s.\n", name,
                  anaFunc->fName);
            status = 0;
            goto end;
        }

        switch (wrasspOptions->optNum) {
        case WO_BEGINTIME:
            if (!cJSON_IsNumber(cjsonp)){
                fprintf(stderr, "No number value for %s\n", cjsonp->string);
                status = 0;
                goto end;
            }
            opt->beginTime = cjsonp->valuedouble;
            break;
        case WO_ENDTIME:
            if (!cJSON_IsNumber(cjsonp)){
                fprintf(stderr, "No number value for %s\n", cjsonp->string);
                status = 0;
                goto end;
            }
            opt->endTime = cjsonp->valuedouble;
            break;
        case WO_CENTRETIME:
            if (cJSON_IsTrue(cjsonp)) {
                opt->options |= AOPT_USE_CTIME;
            } else {
                opt->options &= ~AOPT_USE_CTIME;
            }
            break;

        case WO_MSEFFLEN:
            if (cJSON_IsTrue(cjsonp)) {
                opt->options |= AOPT_EFFECTIVE;
                switch (anaFunc->funcNum) {
                case AF_SPECTRUM:
                    opt->options &= ~AOPT_USE_ENBW;
                    break;
                default:
                    /*
                     * do nothing
                     */
                    break;
                }
            } else {
                opt->options &= ~AOPT_EFFECTIVE;
                switch (anaFunc->funcNum) {
                case AF_FOREST:
                    opt->gender = 'u';
                    break;
                default:
                    break;
                }
            }
            break;
        case WO_MSSIZE:
            if (!cJSON_IsNumber(cjsonp)){
                fprintf(stderr, "No number value for %s\n", cjsonp->string);
                status = 0;
                goto end;
            }
            opt->msSize = cjsonp->valueint;
            switch (anaFunc->funcNum) {
            case AF_FOREST:
                switch (opt->gender) {
                case 'f':
                    if (opt->msSize != FMT_DEF_EFFLENf)
                        opt->gender = 'u';
                    break;
                case 'm':
                    if (opt->msSize != FMT_DEF_EFFLENm)
                        opt->gender = 'u';
                    break;
                default:
                    break;
                }
                break;
            case AF_SPECTRUM:
                opt->options &= ~AOPT_USE_ENBW;
                break;
            default:
                /*
                 * do nothing
                 */
                break;
            }
            break;
        case WO_MSSHIFT:
            // TODO: cJSON_IsNumber check?
            opt->msShift = cjsonp->valuedouble;
            break;
        case WO_MSSMOOTH:
            opt->msSmooth = cjsonp->valuedouble;
            break;
        case WO_BANDWIDTH:
            opt->bandwidth = cjsonp->valuedouble;
            break;
        case WO_RESOLUTION:
            opt->resolution = cjsonp->valuedouble;
            break;
        case WO_GAIN:
            opt->gain = cjsonp->valuedouble;
            break;
        case WO_RANGE:
            opt->range = cjsonp->valuedouble;
            break;
        case WO_PREEMPH:
            opt->preEmph = cjsonp->valuedouble;
            break;
        case WO_FFTLEN:
            opt->FFTLen = cjsonp->valueint;
            break;
        case WO_CHANNEL:
            opt->channel = cjsonp->valueint;
            break;
        case WO_GENDER:
            /*
             * some things need to be set here:
             * for f0_ksv: maxf, minf
             * for f0_mhs: maxf, minf
             * for forest: eff. window length, nominal F1
             */
            gend = gender;
            while (gend->ident != NULL) {
                if (strncmp(gend->ident, cjsonp->valuestring, 1) == 0)
                    break;
                gend++;
            }
            if (gend->ident == NULL){
                fprintf(stderr, "Invalid gender specification %s.\n", cjsonp->valuestring);
                status = 0;
                goto end;
            }
            switch (anaFunc->funcNum) {
            case AF_KSV_PITCH:
                tmp = setKSVgenderDefaults(opt, gend->code);
                break;
            case AF_MHS_PITCH:
                tmp = setMHSgenderDefaults(opt, gend->code);
                break;
            case AF_FOREST:
                if (gend->num == TG_UNKNOWN) {
                    opt->gender = tolower((int) gend->code);
                    tmp = 1;
                } else
                    tmp = setFMTgenderDefaults(opt, gend->code);
                break;
            default:
                tmp = 1;
                break;
            }
            if (tmp < 0){
                fprintf(stderr, "%s", applMessage);
            }
            break;
        case WO_MHS_OPT_POWER:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= MHS_OPT_POWER;
            else
                opt->options &= ~MHS_OPT_POWER;
            break;
        case WO_ORDER:
            tmp = opt->order;
            opt->order = cjsonp->valueint;
            if (anaFunc->funcNum == AF_FOREST) {
                if ((opt->order % 2) != 0) {
                    opt->order = tmp;
                    fprintf(stderr, "Prediction order must be an even number.");
                    status = 0;
                    goto end;
                } else {
                    opt->options |= FMT_OPT_LPO_FIXED;
                    opt->increment = 0;
                }
            }
            break;
        case WO_INCREMENT:
            opt->increment = cjsonp->valueint;
            if (anaFunc->funcNum == AF_FOREST) {
                opt->options &= ~FMT_OPT_LPO_FIXED;
                opt->order = 0;
            }
            break;
        case WO_NUMLEVELS:
            opt->numLevels = cjsonp->valueint;
            break;
        case WO_NUMFORMANTS:
            opt->numFormants = cjsonp->valueint;
            break;
        case WO_PRECISION:
            opt->precision = cjsonp->valueint;
            break;
        case WO_ACCURACY:
            opt->accuracy = cjsonp->valueint;
            break;
        case WO_ALPHA:
            opt->alpha = cjsonp->valuedouble;
            break;
        case WO_THRESHOLD:
            opt->threshold = cjsonp->valuedouble;
            break;
        case WO_MAXF:
            opt->maxF = cjsonp->valuedouble;
            switch (anaFunc->funcNum) {
            case AF_KSV_PITCH:
            case AF_MHS_PITCH:
                opt->gender = 'u';
                break;
            default:
                /*
                 * do nothing
                 */
                break;
            }
            break;
        case WO_MINF:
            opt->minF = cjsonp->valuedouble;
            switch (anaFunc->funcNum) {
            case AF_KSV_PITCH:
            case AF_MHS_PITCH:
                opt->gender = 'u';
                break;
            default:
                /*
                 * do nothing
                 */
                break;
            }
            break;
        case WO_NOMF1:         /* e.g. for formant analysis */
            opt->nomF1 = cjsonp->valuedouble;
            switch (anaFunc->funcNum) {
            case AF_FOREST:
                switch (opt->gender) {
                case 'f':
                    if (opt->nomF1 != FMT_DEF_NOMF1f)
                        opt->gender = 'u';
                    break;
                case 'm':
                    if (opt->nomF1 != FMT_DEF_NOMF1m)
                        opt->gender = 'u';
                    break;
                default:
                    break;
                }
                break;
            default:
                /*
                 * do nothing
                 */
                break;
            }
            break;
        case WO_INS_EST:
            if (cjsonp->valueint)
                opt->options |= FMT_OPT_INS_ESTS;
            else
                opt->options &= ~FMT_OPT_INS_ESTS;
            break;
        case WO_VOIAC1PP:      /* VOICING thresholds */
            opt->voiAC1 = cjsonp->valuedouble;
            break;
        case WO_VOIMAG:
            opt->voiMag = cjsonp->valuedouble;
            break;
        case WO_VOIPROB:
            opt->voiProb = cjsonp->valuedouble;
            break;
        case WO_VOIRMS:
            opt->voiRMS = cjsonp->valuedouble;
            break;
        case WO_VOIZCR:
            opt->voiZCR = cjsonp->valuedouble;
            break;
        case WO_HPCUTOFF:      /* filter parameters */
            opt->hpCutOff = cjsonp->valuedouble;
            if (expExt == 0)
                tmp = getFILTtype(opt, anaFunc->defExt);
            break;
        case WO_LPCUTOFF:
            opt->lpCutOff = cjsonp->valuedouble;
            if (expExt == 0)
                tmp = getFILTtype(opt, anaFunc->defExt);
            break;
        case WO_STOPDB:
            opt->stopDB = cjsonp->valuedouble;
            if (expExt == 0)
                tmp = getFILTtype(opt, anaFunc->defExt);
            break;
        case WO_TBWIDTH:
            opt->tbWidth = cjsonp->valuedouble;
            if (expExt == 0)
                tmp = getFILTtype(opt, anaFunc->defExt);
            break;
        case WO_USEIIR:
            if (cjsonp->valueint)
                opt->options |= FILT_OPT_USE_IIR;
            else
                opt->options &= ~FILT_OPT_USE_IIR;
            if (expExt == 0)
                tmp = getFILTtype(opt, anaFunc->defExt);
            break;
        case WO_NUMIIRSECS:
            opt->order = cjsonp->valueint;
            if (opt->order < 1) {
                fprintf(stderr, "Bad value for option -numIIRsections (%i), must be greater 0 (default 4).",
                     opt->order);
                opt->order = FILT_DEF_SECTS;
                status = 0;
                goto end;
            }
            break;
        case WO_TYPE:          /* hold-all */
            switch (anaFunc->funcNum) {
            case AF_RFCANA:
                lPtr = lpType;
                while (lPtr->ident != NULL) {
                    if (strcmp(lPtr->ident, cjsonp->valuestring) == 0)
                        break;
                    lPtr++;
                }
                if (lPtr->ident == NULL){
                    fprintf(stderr, "Invalid LP Type: %s.", cjsonp->valuestring);
                    status = 0;
                    goto end;
                }
                strncpy(opt->type, lPtr->ident, (sizeof opt->type) - 1);
                if (expExt == 0)
                    strncpy(ext, lPtr->ext, (sizeof ext) - 1);
                break;
            case AF_SPECTRUM:
                sPtr = spectType;
                while (sPtr->ident != NULL) {
                    if (strcmp(sPtr->ident, cjsonp->valuestring) == 0)
                        break;
                    sPtr++;
                }
                if (sPtr->ident == NULL){
                    fprintf(stderr, "Invalid SP Type: %s.", cjsonp->valuestring);
                    status = 0;
                    goto end;
                }
                strncpy(opt->type, sPtr->ident, (sizeof opt->type) - 1);
                if (setSPECTdefaults(opt) < 0) {
                    fprintf(stderr, "%s", getAsspMsg(asspMsgNum));
                    status = 0;
                    goto end;
                }
                strncpy(opt->type, sPtr->ident, (sizeof opt->type) - 1);
                switch (sPtr->type) {
                case DT_FTPOW:
                case DT_FTAMP:
                case DT_FTSQR:
                    setDFTdefaults(opt);
                    break;
                case DT_FTLPS:
                    setLPSdefaults(opt);
                    break;
                case DT_FTCSS:
                    setCSSdefaults(opt);
                    break;
                case DT_FTCEP:
                    setCEPdefaults(opt);
                    break;
                default:
                    setAsspMsg(AEG_ERR_BUG,
                               "setSPECTdefaults: invalid default type");
                    fprintf(stderr, "%s.", getAsspMsg(asspMsgNum));
                    status = 0;
                    goto end;
                    break;
                }
                if (expExt == 0)
                    strncpy(ext, sPtr->ext, (sizeof ext) - 1);
                break;
            default:
                break;
            }
            break;
        case WO_WINFUNC:
            wPtr = wfLongList;
            while (wPtr->code != NULL) {
                if (strcmp(wPtr->code, cjsonp->valuestring) == 0)
                    break;
                wPtr++;
            }
            if (wPtr->code == NULL){
                fprintf(stderr, "Invalid window function code %s.",
                      cjsonp->valuestring);
                status = 0;
                goto end;
            }
            strncpy(opt->winFunc, wPtr->code, (sizeof opt->winFunc) - 1);
            break;
            /*
             * These are not in libassp but in wrassp
             */
        case WO_ENERGYNORM:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= ACF_OPT_NORM;
            else
                opt->options &= ~ACF_OPT_NORM;
            break;
        case WO_LENGTHNORM:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= ACF_OPT_MEAN;
            else
                opt->options &= ~ACF_OPT_MEAN;
            break;
        case WO_DIFF_OPT_BACKWARD:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= DIFF_OPT_BACKWARD;
            else
                opt->options &= ~DIFF_OPT_BACKWARD;
            break;
        case WO_DIFF_OPT_CENTRAL:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= DIFF_OPT_CENTRAL;
            else
                opt->options &= ~DIFF_OPT_CENTRAL;
            break;
        case WO_RMS_OPT_LINEAR:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= RMS_OPT_LINEAR;
            else
                opt->options &= ~RMS_OPT_LINEAR;
            break;
        case WO_LPS_OPT_DEEMPH:
            if (cJSON_IsTrue(cjsonp))
                opt->options |= LPS_OPT_DEEMPH;
            else
                opt->options &= ~LPS_OPT_DEEMPH;
            break;
        case WO_OUTPUTEXT:
            // not allowed in wasmassp
            break;
            // if (TYPEOF(el) == NILSXP) {
            //     expExt = 0;
            //     break;
            // }
            // cPtr = strdup(CHAR(STRING_ELT(el, 0)));
            // if (*cPtr != '.' && strlen(cPtr) != 0) {
            //     strncpy(ext, ".", strlen(".") + 1);
            //     strcat(ext, cPtr);
            // } else {
            //     strncpy(ext, cPtr,  (sizeof ext) - 1);
            // }
            // free(cPtr);
            // expExt = 1;
            // switch (anaFunc->funcNum) {
            // case AF_RFCANA:
            //     lPtr = lpType;
            //     while (lPtr->ident != NULL) {
            //         if (strcmp(opt->type, lPtr->ident) == 0)
            //             break;
            //         lPtr++;
            //     }
            //     if (lPtr->ident == NULL)
            //         error("Bad LP Type in memory (%s).", opt->type);
            //     if (strcmp(lPtr->ext, ext) == 0) {
            //         expExt = 0;
            //     } else {
            //         expExt = 1;
            //     }
            //     break;
            // case AF_SPECTRUM:
            //     sPtr = spectType;
            //     while (sPtr->ident != NULL) {
            //         if (strcmp(opt->type, sPtr->ident) == 0)
            //             break;
            //         sPtr++;
            //     }
            //     if (sPtr->ident == NULL)
            //         error("Bad SP Type in memory (%s).", opt->type);
            //     if (strcmp(sPtr->ext, ext) == 0) {
            //         expExt = 0;
            //     } else {
            //         expExt = 1;

            //     }
            //     break;
            // default:
            //     break;
            // }
            break;
        case WO_TOFILE:
            // not allowed in wasmassp
            // toFile = INTEGER(el)[0] != 0;
            break;
        case WO_OUTPUTDIR:
            // not allowed in wasmassp
            // if (el == R_NilValue) {
            //     outDir = NULL;
            //     break;
            // }
            // outDir = strdup(CHAR(STRING_ELT(el, 0)));
            // if (outDir[strlen(outDir) - 1] != DIR_SEP_CHR) {
            //     /* add trailing slash, but we need a bit more space first */
            //     char *tmp = malloc(strlen(outDir) + 2);
            //     strcpy(tmp, outDir);
            //     tmp = strcat(tmp, DIR_SEP_STR);
            //     free(outDir);
            //     outDir = tmp;
            // }
            break;
        case WO_PBAR:
            // not allowed in wasmassp
            // pBar = el;
            break;
        default:
            break;
        }
        // move pointer 4ward
        cjsonp = cjsonp->next;
    }



    /*
     * open
     */
    inPtr = asspFOpen(strdup(audio_path), AFO_READ, (DOBJ *) NULL);
    if (inPtr == NULL){
        fprintf(stderr, "%s (%s)\n", getAsspMsg(asspMsgNum), strdup(audio_path));
        return 1;
    }

    /*
     * run the function (as pointed to in the descriptor) to generate
     * the output object 
     */
    outPtr = (anaFunc->compProc) (inPtr, opt, (DOBJ *) NULL);
    if (outPtr == NULL) {
        asspFClose(inPtr, AFC_FREE);
        fprintf(stderr, "%s (%s)\n", getAsspMsg(asspMsgNum), strdup(audio_path));
        return 1;
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
    
    end:
        cJSON_Delete(json);
        return status;
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

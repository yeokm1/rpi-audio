/*

This example reads from the default PCM device
and writes to standard output for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <math.h>

//from http://web.mit.edu/~emin/www.old/source_code/fft/
#include <stdio.h>
#define SIN_2PI_16 0.38268343236508978
#define SIN_4PI_16 0.707106781186547460
#define SIN_6PI_16 0.923879532511286740
#define C_P_S_2PI_16 1.30656296487637660
#define C_M_S_2PI_16 0.54119610014619690
#define C_P_S_6PI_16 1.3065629648763766
#define C_M_S_6PI_16 -0.54119610014619690

/* INPUT: float input[16], float output[16] */
/* OUTPUT: none */
/* EFFECTS:  Places the 16 point fft of input in output in a strange */
/* order using 10 real multiplies and 79 real adds. */
/* Re{F[0]}= out0 */
/* Im{F[0]}= 0 */
/* Re{F[1]}= out8 */
/* Im{F[1]}= out12 */
/* Re{F[2]}= out4 */
/* Im{F[2]}= -out6 */
/* Re{F[3]}= out11 */
/* Im{F[3]}= -out15 */
/* Re{F[4]}= out2 */
/* Im{F[4]}= -out3 */
/* Re{F[5]}= out10 */
/* Im{F[5]}= out14 */
/* Re{F[6]}= out5 */
/* Im{F[6]}= -out7 */
/* Re{F[7]}= out9 */
/* Im{F[7]}= -out13 */
/* Re{F[8]}= out1 */
/* Im{F[8]}=0 */
/* F[9] through F[15] can be found by using the formula */
/* Re{F[n]}=Re{F[(16-n)mod16]} and Im{F[n]}= -Im{F[(16-n)mod16]} */

/* Note using temporary variables to store intermediate computations */
/* in the butterflies might speed things up.  When the current version */
/* needs to compute a=a+b, and b=a-b, I do a=a+b followed by b=a-b-b.  */
/* So practically everything is done in place, but the number of adds */
/* can be reduced by doinc c=a+b followed by b=a-b. */

/* The algorithm behind this program is to find F[2k] and F[4k+1] */
/* seperately.  To find F[2k] we take the 8 point Real FFT of x[n]+x[n+8] */
/* for n from 0 to 7.  To find F[4k+1] we take the 4 point Complex FFT of */
/* exp(-2*pi*j*n/16)*{x[n] - x[n+8] + j(x[n+12]-x[n+4])} for n from 0 to 3.*/

void R16SRFFT(float input[16],float output[16] ) {
  float temp, out0, out1, out2, out3, out4, out5, out6, out7, out8;
  float out9,out10,out11,out12,out13,out14,out15;

  out0=input[0]+input[8]; /* output[0 through 7] is the data that we */
  out1=input[1]+input[9]; /* take the 8 point real FFT of. */
  out2=input[2]+input[10];
  out3=input[3]+input[11];
  out4=input[4]+input[12];
  out5=input[5]+input[13];
  out6=input[6]+input[14];
  out7=input[7]+input[15];



  out8=input[0]-input[8];   /* inputs 8,9,10,11 are */
  out9=input[1]-input[9];   /* the Real part of the */
  out10=input[2]-input[10]; /* 4 point Complex FFT inputs.*/
  out11=input[3]-input[11]; 
  out12=input[12]-input[4]; /* outputs 12,13,14,15 are */
  out13=input[13]-input[5]; /* the Imaginary pars of  */
  out14=input[14]-input[6]; /* the 4 point Complex FFT inputs.*/
  out15=input[15]-input[7];

  /*First we do the "twiddle factor" multiplies for the 4 point CFFT */
  /*Note that we use the following handy trick for doing a complex */
  /*multiply:  (e+jf)=(a+jb)*(c+jd) */
  /*   e=(a-b)*d + a*(c-d)   and    f=(a-b)*d + b*(c+d)  */

  /* C_M_S_2PI/16=cos(2pi/16)-sin(2pi/16) when replaced by macroexpansion */
  /* C_P_S_2PI/16=cos(2pi/16)+sin(2pi/16) when replaced by macroexpansion */
  /* (SIN_2PI_16)=sin(2pi/16) when replaced by macroexpansion */
  temp=(out13-out9)*(SIN_2PI_16); 
  out9=out9*(C_P_S_2PI_16)+temp; 
  out13=out13*(C_M_S_2PI_16)+temp;
  
  out14*=(SIN_4PI_16);
  out10*=(SIN_4PI_16);
  out14=out14-out10;
  out10=out14+out10+out10;
  
  temp=(out15-out11)*(SIN_6PI_16);
  out11=out11*(C_P_S_6PI_16)+temp;
  out15=out15*(C_M_S_6PI_16)+temp;

  /* The following are the first set of two point butterfiles */
  /* for the 4 point CFFT */

  out8+=out10;
  out10=out8-out10-out10;

  out12+=out14;
  out14=out12-out14-out14;

  out9+=out11;
  out11=out9-out11-out11;

  out13+=out15;
  out15=out13-out15-out15;

  /*The followin are the final set of two point butterflies */
  output[1]=out8+out9;
  output[7]=out8-out9;

  output[9]=out12+out13;
  output[15]=out13-out12;
  
  output[5]=out10+out15;        /* implicit multiplies by */
  output[13]=out14-out11;        /* a twiddle factor of -j */                            
  output[3]=out10-out15;  /* implicit multiplies by */
  output[11]=-out14-out11;  /* a twiddle factor of -j */

  
  /* What follows is the 8-point FFT of points output[0-7] */
  /* This 8-point FFT is basically a Decimation in Frequency FFT */
  /* where we take advantage of the fact that the initial data is real*/

  /* First set of 2-point butterflies */
    
  out0=out0+out4;
  out4=out0-out4-out4;
  out1=out1+out5;
  out5=out1-out5-out5;
  out2+=out6;
  out6=out2-out6-out6;
  out3+=out7;
  out7=out3-out7-out7;

  /* Computations to find X[0], X[4], X[6] */
  
  output[0]=out0+out2;
  output[4]=out0-out2;
  out1+=out3;
  output[12]=out3+out3-out1;

  output[0]+=out1;  /* Real Part of X[0] */
  output[8]=output[0]-out1-out1; /*Real Part of X[4] */
  /* out2 = Real Part of X[6] */
  /* out3 = Imag Part of X[6] */
  
  /* Computations to find X[5], X[7] */

  out5*=SIN_4PI_16;
  out7*=SIN_4PI_16;
  out5=out5-out7;
  out7=out5+out7+out7;

  output[14]=out6-out7; /* Imag Part of X[5] */
  output[2]=out5+out4; /* Real Part of X[7] */
  output[6]=out4-out5; /*Real Part of X[5] */
  output[10]=-out7-out6; /* Imag Part of X[7] */

}

int main(int argc, char * argv[]) {
  long capture_loops;
  int capture_rc;
  int capture_size;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *capture_params;
  unsigned int sample_rate;
  int capture_dir;
  snd_pcm_uframes_t capture_frames;
  char *capture_buffer;




  /* Open PCM device for recording (capture). */
  capture_rc = snd_pcm_open(&capture_handle, argv[1],
                    SND_PCM_STREAM_CAPTURE, 0);
  if (capture_rc < 0) {
    fprintf(stderr,
            "unable to open record pcm device: %s\n",
            snd_strerror(capture_rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&capture_params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(capture_handle, capture_params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(capture_handle, capture_params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(capture_handle, capture_params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(capture_handle, capture_params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  sample_rate = 44100;
  snd_pcm_hw_params_set_rate_near(capture_handle, capture_params,
                                  &sample_rate, &capture_dir);

  /* Set period size to 32 frames. */
  capture_frames = 2;
  snd_pcm_hw_params_set_period_size_near(capture_handle,
                              capture_params, &capture_frames, &capture_dir);

  /* Write the parameters to the driver */
  capture_rc = snd_pcm_hw_params(capture_handle, capture_params);
  if (capture_rc < 0) {
    fprintf(stderr,
            "unable to set input hw parameters: %s\n",
            snd_strerror(capture_rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(capture_params,
                                      &capture_frames, &capture_dir);
  capture_size = capture_frames * 4; /* 2 bytes/sample, 2 channels */
  capture_buffer = (char *) malloc(capture_size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(capture_params,
                                         &sample_rate, &capture_dir);
  capture_loops = 5000000 / sample_rate;


  //Output init

  int output_rc;
  int output_size;
  snd_pcm_t *output_handle;
  snd_pcm_hw_params_t *output_params;
  unsigned int output_val;
  int output_dir;
  snd_pcm_uframes_t output_frames;
  char *output_buffer;

  /* Open PCM device for playback. */
  output_rc = snd_pcm_open(&output_handle, argv[2],
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (output_rc < 0) {
    fprintf(stderr,
            "unable to open output pcm device: %s\n",
            snd_strerror(output_rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&output_params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(output_handle, output_params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(output_handle, output_params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(output_handle, output_params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(output_handle, output_params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  output_val = 44100;
  snd_pcm_hw_params_set_rate_near(output_handle, output_params,
                                  &output_val, &output_dir);

  /* Set period size to 32 frames. */
  output_frames = 32;
  snd_pcm_hw_params_set_period_size_near(output_handle,
                              output_params, &output_frames, &output_dir);

  /* Write the parameters to the driver */
  output_rc = snd_pcm_hw_params(output_handle, output_params);
  if (output_rc < 0) {
    fprintf(stderr,
            "unable to set output hw parameters: %s\n",
            snd_strerror(output_rc));
    exit(1);
  }


  //DIY DNC stuffs
  
  int look_back_length = 3;

  float prevRms = 0;
  float targetRms = 0.85;
  float currentRms = 0;

  //previous S_max
  float prev_gain[3];
  int oldestHistory = 0;

  int firstRun = 1;

  //amount of gain to add
  float gain; 
  float maxGain = 5;
  //max magnitude of frame
  float S_max = 0;
  //max magnitude of peak
  float Peak = 0.95f;
  //G[n]=Peak/abs(S_max[n])
  
  /*
  const uint32_t channels = 2;
  const uint32_t sampleRate = 44100;
  const uint32_t frameLenMsec = 100;
  const uint32_t filterSize = 15; //default 31
  const double peakValue = 0.95f; //0.95 default
  const double maxAmplification = 10f;
  //user input
  const double targetRms = 0.8f; // range from 0 to 1

  const bool channelsCoupled = true;
  const bool enableDCCorrection = false;
  const bool altBoundaryMode = false;
  FILE *const logFile = NULL;

  MDynamicAudioNormalizer_Handle* mdanHandle = createInstance(channels,sampleRate, frameLenMsec,filterSize,peakValue,maxAmplification, targetRms, compressFactor, channelsCoupled, enableDCCorrection, altBoundaryMode, logFile);
  
  */

  prev_gain[0] = 0;
  prev_gain[1] = 0;
  prev_gain[2] = 0;
  double inputSamples[2][capture_size/2];

  //extract left and right channels into their buffers
  //temp buffer for conversion to float
  char temp[2];

  float currentGain = 0;

   char small ='0';
  char big = '0';

  signed short int tempSum = 0;

  char output[capture_size];
  float sum = 0;

  float rmsDiff = 0;

  float previousVals[16];

  int cumFrameNum = 0;

  while (1) {

    capture_rc = snd_pcm_readi(capture_handle, capture_buffer, capture_frames);
    if (capture_rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(capture_handle);
    } else if (capture_rc < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(capture_rc));
    } else if (capture_rc != (int)capture_frames) {
      fprintf(stderr, "short read, read %d frames\n", capture_rc);
    }

    //output_rc = snd_pcm_writei(output_handle, capture_buffer, capture_frames);

    /*
    char leftChannel[(capture_size/2)];
    char rightChannel[(capture_size/2)];
    double leftBuffer[(capture_size/2)];
    double rightBuffer[(capture_size/2)];

    double inputSamples[2][capture_size/2];
    */

    S_max = 0;
    for(int i = 0; i < capture_frames; i++){
      /*
        leftChannel[i] = capture_buffer[(i*4)];
        leftChannel[i] = capture_buffer[(i*4)+1];
        rightChannel[i] = capture_buffer[(i*4)+2];
        rightChannel[i] = capture_buffer[(i*4)+3];
      */
      temp[0] = capture_buffer[i*4];
      temp[1] = capture_buffer[i*4+1];
      inputSamples[0][i] = (double)(temp[0] + temp[1]*256 - 32767) / 32768;

      temp[0] = capture_buffer[i*4+2];
      temp[1] = capture_buffer[i*4+3];
      inputSamples[1][i] = (double)(temp[0] + temp[1]*256 - 32767) / 32768;

      if(cumFrameNum < 16){
        previousVals[cumFrameNum] = inputSamples[0][i];
        cumFrameNum++;
         previousVals[cumFrameNum] = inputSamples[0][i];
         cumFrameNum++;
      }

      //sanitization

      if(inputSamples[0][i] > 1){
        inputSamples[0][i] = 1;
      }
      else if(inputSamples[0][i] < -1){
      }

      //sanitization
      if(inputSamples[1][i] > 1){
        inputSamples[1][i] = 1;
      }
      else if(inputSamples[1][i] < -1){
        inputSamples[1][i] = -1;
      }

      if(inputSamples[0][i] > S_max){
          S_max = inputSamples[0][i];
      }
      if( abs(inputSamples[1][i]) > S_max){
          S_max = abs(inputSamples[1][i]);
      }

    }

    gain = Peak / S_max;
    
    if(gain > maxGain){
      gain = maxGain;
    }
    
    //printf("%f\n", gain);
    /*reference
    short* sampleBuffer;
    ...
    short sample=*sampleBuffer;
    double dsample=(double)sample * gain;
    if (dsample>32767.0) {dsample=32767.0;}
    if (dsample<-32768.0) {dsample=-32768.0;}
    *sampleBuffer=(short)dsample;
    sampleBuffer++;

    */

    //smoothen the gain
    currentGain = gain;

    gain = (9.0f/16.0f)*gain + (1.0f/16.0f)*prev_gain[0] + (1.0f/8.0f)*prev_gain[1] + (1.0f/4.0f)*prev_gain[2];
   

    //apply the gain
    for(int i = 0; i < capture_frames; i++){
      if(inputSamples[0][i] > 0.5)
        inputSamples[0][i] *= gain;
      if(inputSamples[1][i] > 0.5)
        inputSamples[1][i] *= gain;
      for(int j = 0; j < 2; j++){
        if(inputSamples[j][i] > 1){
          inputSamples[j][i] = 1;
        }
        else if(inputSamples[j][i] < -1){
          inputSamples[j][i] = -1;
        }
      }

    }
    
    for(int i = 0; i < 3; i++){
      if(i+1 < 3){
        prev_gain[i] = prev_gain[i+1];
      }
      else{
        prev_gain[i] = currentGain;
      }

    }

    sum = 0;
    //limit the rms of the entire buffer
    for(int i = 0; i < capture_frames; i++){
      sum += inputSamples[0][i] * inputSamples[0][i];
      sum += inputSamples[1][i] * inputSamples[1][i];
    }

    currentRms = sqrt( sum/capture_size );

    //nerfing with rms
    rmsDiff = targetRms - currentRms;
    if(rmsDiff > 0){
      for(int i = 0; i < capture_frames; i++){
        inputSamples[0][i] *= (1.0 + rmsDiff);
        inputSamples[1][i] *= (1.0 + rmsDiff);

           //sanitization

      if(inputSamples[0][i] > 1){
        inputSamples[0][i] = 1;
      }
      else if(inputSamples[0][i] < -1){
      }

      //sanitization
      if(inputSamples[1][i] > 1){
        inputSamples[1][i] = 1;
      }
      else if(inputSamples[1][i] < -1){
        inputSamples[1][i] = -1;
      }

      }

    }
    else if(rmsDiff < 0){
      for(int i = 0; i < capture_frames; i++){
        inputSamples[0][i] *= (1.0 + rmsDiff);
        inputSamples[1][i] *= (1.0 + rmsDiff);

           //sanitization

      if(inputSamples[0][i] > 1){
        inputSamples[0][i] = 1;
      }
      else if(inputSamples[0][i] < -1){
      }

      //sanitization
      if(inputSamples[1][i] > 1){
        inputSamples[1][i] = 1;
      }
      else if(inputSamples[1][i] < -1){
        inputSamples[1][i] = -1;
      }

      }
    }



    //convert from 1.0 to -1 back to 2byte units
    //little endian
    tempSum = 0;

    for(int i = 0; i < capture_frames; i++){
      //left
      tempSum = (inputSamples[0][i]* 32768) + 32767;

      small = tempSum%256;
      big = tempSum/256;
      
      output[i*4] = small;
      output[i*4+1] = big;

      //right
      tempSum = (inputSamples[1][i] * 32768) + 32767;

      small = tempSum%256;
      big = tempSum/256;
      
      output[i*4+2] = small;
      output[i*4+3] = big;

    }

    output_rc = snd_pcm_writei(output_handle, output, capture_frames);
    
    if (output_rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(output_handle);
    } else if (output_rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(output_rc));
    }  else if (output_rc != (int)capture_frames) {
      fprintf(stderr,
              "short write, write %d frames\n", output_rc);
    }


  }

  snd_pcm_drain(output_handle);
  snd_pcm_close(output_handle);
  //free(output_buffer);


  snd_pcm_drain(capture_handle);
  snd_pcm_close(capture_handle);
  free(capture_buffer);

  return 0;
}

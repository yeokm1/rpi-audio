/*

This example reads from the default PCM device
and writes to standard output for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <math.h>

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
  capture_frames = 8;
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
  output_frames = capture_frames;
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
  float targetRms = 0.9;
  float currentRms = 0;

  //previous S_max

  int firstRun = 1;

  //amount of gain to add
  float gain; 
  float maxGain = 10;
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
  float prev_gain[3];
  prev_gain[0] = 0;
  prev_gain[1] = 0;
  prev_gain[2] = 0;

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
    double inputSamples[2][capture_size/2];

    //extract left and right channels into their buffers
    //temp buffer for conversion to float
    char temp[2];
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
      inputSamples[0][i] = (double)(temp[0] + temp[1]*256 - 32767)/ 32767;

      temp[0] = capture_buffer[i*4+2];
      temp[1] = capture_buffer[i*4+3];
      inputSamples[1][i] = (double)(temp[0] + temp[1]*256 - 32767)/ 32767;

      //sanitization
      if(inputSamples[0][i] > 1){
        inputSamples[0][i] = 1;
      }
      else if(inputSamples[0][i] < -1){
        inputSamples[0][i] = -1;
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

    //smoothen the gain

    //prev_gain = gain;


     //smoothen the gain
    float currentGain = gain;

    //gain = (9.0f/16.0f)*gain + (1.0f/16.0f)*prev_gain[0] + (1.0f/8.0f)*prev_gain[1] + (1.0f/4.0f)*prev_gain[2];
   

    //apply the gain
    for(int i = 0; i < capture_frames; i++){
      if(inputSamples[0][i] > 0.2)
        inputSamples[0][i] *= gain;
      if(inputSamples[1][i] > 0.2)
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

    float sum = 0;
    //limit the rms of the entire buffer
    for(int i = 0; i < capture_frames; i++){
      sum += inputSamples[0][i] * inputSamples[0][i];
      sum += inputSamples[1][i] * inputSamples[1][i];
    }

    float currentRms = sqrt( sum/capture_size );
    int rms = 0;
    //nerfing with rms
    if(rms){
    float rmsDiff = targetRms - currentRms;
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
  }


    //limit the rms of the entire buffer
    //currentRms = sqrt(    );

    //convert from 1.0 to -1 back to 2byte units
    //little endian
    unsigned short int tempSum = 0;

    char output[capture_size];

    for(int i = 0; i < capture_frames; i++){
      //left
      tempSum = (inputSamples[0][i]*32767) + 32767;

      char small = tempSum%256;
      char big = tempSum/256;
      
      output[i*4] = small;
      output[i*4+1] = big;

      //right
      tempSum = (inputSamples[1][i]*32767) + 32767;

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
  free(output_buffer);


  // snd_pcm_drain(capture_handle);
  // snd_pcm_close(capture_handle);
  // free(capture_buffer);

  return 0;
}

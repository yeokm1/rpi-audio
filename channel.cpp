/*

This example reads from the default PCM device
and writes to standard output for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

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
                              SND_PCM_FORMAT_S16_LE );

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(capture_handle, capture_params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  sample_rate = 44100;
  snd_pcm_hw_params_set_rate_near(capture_handle, capture_params,
                                  &sample_rate, &capture_dir);

  /* Set period size to 32 frames. */
  capture_frames = 256;
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

  double prevRms = 0;
  double targetRms = 0.5;
  double currentRms = 0;

  //previous S_max

  int firstRun = 1;

  //amount of gain to add
  double gain; 
  double maxGain = 10;
  //max magnitude of frame
  double S_max = 1;
  //max magnitude of peak
  double Peak = 0.8;
  //G[n]=Peak/abs(S_max[n])

   double prev_gain[3];
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
    //temp buffer for conversion to double
    char temp[2];
    S_max = 0.0000000001;
    gain = 1.0;
    for(int i = 0; i < capture_frames; i++){
      /*
        leftChannel[i] = capture_buffer[(i*4)];
        leftChannel[i] = capture_buffer[(i*4)+1];
        rightChannel[i] = capture_buffer[(i*4)+2];
        rightChannel[i] = capture_buffer[(i*4)+3];
      */


	signed short tempVal;

      temp[0] = capture_buffer[i*4];
      temp[1] = capture_buffer[i*4+1];
      tempVal = (*(signed short*) temp);
      inputSamples[0][i] = (double) tempVal/ 32768;


      temp[0] = capture_buffer[i*4+2];
      temp[1] = capture_buffer[i*4+3];
      tempVal = (*(signed short*) temp);
      inputSamples[1][i] = (double) tempVal/ 32768;
    

/*
      cout << i << " frame: " << (int)capture_buffer[i*4] << " " << (int)capture_buffer[i*4+1] << " "  << (int)capture_buffer[i*4+2] << " " << (int)capture_buffer[i*4+3] << " " << 
      inputSamples[0][i] << " " << inputSamples[1][i] << " "  << endl;
*/
      
  //    cout << inputSamples[0][i] << " " << inputSamples[1][i] << endl;


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

      if(abs(inputSamples[0][i]) > S_max){
          S_max = abs(inputSamples[0][i]);
      }
      if( abs(inputSamples[1][i]) > S_max){
          S_max = abs(inputSamples[1][i]);
      }

    }

     //preliminary boosting
    for(int i = 0; i < capture_frames; i++){

      if(S_max < 0.5){

          if(abs(inputSamples[0][i]) > 0.05){
            if(inputSamples[0][i] > 0)
            inputSamples[0][i] += 0.3;
            else{
              inputSamples[0][i] -= 0.3;
            }
          }
          if(abs(inputSamples[1][i]) > 0.05){
            if(inputSamples[1][i] > 0)
            inputSamples[1][i] += 0.3;
            else{
              inputSamples[1][i] -= 0.3;
            }
          }
      }
      else{

/*
         if( abs(inputSamples[0][i]) < 0.6){
          inputSamples[0][i] *= 1.2;
        }
        if( abs(inputSamples[1][i]) < 0.6){
          inputSamples[1][i] *= 1.2;
        }
      */

      if(S_max > 0.8){

        if(abs(inputSamples[0][i] > 0.3)){
            if(inputSamples[0][i] > 0)
            inputSamples[0][i] -= 0.1;
            else{
              inputSamples[0][i] += 0.1;
            }
		}
        else if(abs(inputSamples[0][i]) > 0.5){
            if(inputSamples[0][1] > 0)
              inputSamples[0][i] -= 0.125;
            else{
              inputSamples[0][i] += 0.125;
            }
          }
          else if(inputSamples[0][i] > 0.8){
            if(inputSamples[0][i] > 0)
              inputSamples[0][i] -= 0.15;
            else{
              inputSamples[0][i] += 0.15;
            }
          }

        if(abs(inputSamples[1][i]) > 0.3){
            if(inputSamples[1][i] > 0)
            inputSamples[1][i] -= 0.1;
            else{
              inputSamples[1][i] += 0.1;
            }
		}
        else if(abs(inputSamples[1][i]) > 0.5){
            if(inputSamples[1][1] > 0)
              inputSamples[1][i] -= 0.125;
            else{
              inputSamples[1][i] += 0.125;
            }
          }
          else if(inputSamples[1][i] > 0.8){
            if(inputSamples[1][i] > 0)
              inputSamples[1][i] -= 0.15;
            else{
              inputSamples[1][i] += 0.15;
            }
          }
		  
        }
		
	      //secondary boosting
     if( abs(inputSamples[0][i]) > targetRms){
        inputSamples[0][i] = 0.8 * inputSamples[0][i];
      }
      if( abs(inputSamples[1][i]) > targetRms){
        inputSamples[1][i] = 0.8 * inputSamples[1][i];
      }
      
		
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

      }

      }


     // inputSamples[0][i] = (inputSamples[0][i] ) * Peak;
      //inputSamples[1][i] = (inputSamples[1][i] ) * Peak;


      /*
      if( abs(inputSamples[0][i]) > 0.05 && abs(inputSamples[0][i]) < 0.6){
        inputSamples[0][i] *= 1.5;
      }
      if( abs(inputSamples[1][i]) > 0.05 && abs(inputSamples[0][i]) < 0.6){
        inputSamples[1][i] *= 1.5;
      }

      if(abs(inputSamples[0][i]) > 0.05 && abs(inputSamples[0][i]) < 0.2 ){
         inputSamples[0][i] += 0.2;
      }

      if(abs(inputSamples[1][i]) > 0.05 && abs(inputSamples[1][i]) < 0.2 ){
         inputSamples[1][i] += 0.2;
      }
      */

/*

    gain = Peak / S_max;



    if(gain > 500){
      gain = 0;
      //printf("WHATTT???\n");
    } 


    else if (gain < 50){
      gain = 50;
    } else if (gain < 100){
      gain = 100;
    }
  
   //


    double currentGain = gain;

    //gain = (9.0/16.0)*gain + (1.0/16.0)*prev_gain[0] + (1.0/8.0)*prev_gain[1] + (1.0/4.0)*prev_gain[2];
    gain = (23.0/32.0)*gain + (1.0/32.0)*prev_gain[0] + (3.0/32.0)*prev_gain[1] + (5.0/4320)*prev_gain[2];

    for(int i = 0; i < 3; i++){
      if(i+1 < 3){
        prev_gain[i] = prev_gain[i+1];
      }
      else{
        prev_gain[i] = currentGain;
      }

    }
    */

	
      /*
    //apply the gain
    for(int i = 0; i < capture_frames; i++){

      if(abs(inputSamples[0][i]) > 0.05 && abs(inputSamples[0][i]) < 0.4 )
        inputSamples[0][i] *= gain * 2;
      if(abs(inputSamples[1][i]) > 0.05 && abs(inputSamples[0][i]) < 0.4)
        inputSamples[1][i] *= gain * 2;

      if(abs(inputSamples[0][i]) > 0.4 && abs(inputSamples[0][i]) < 0.7)
        inputSamples[0][i] *= gain * 1.2;
      if(abs(inputSamples[1][i]) > 0.4 && abs(inputSamples[0][i]) < 0.7)
        inputSamples[1][i] *= gain * 1.2;

      if(abs(inputSamples[0][i]) > 0.7 && abs(inputSamples[0][i]) < 0.85)
        inputSamples[0][i] *= gain * 0.8;
      if(abs(inputSamples[1][i]) > 0.7 && abs(inputSamples[0][i]) < 0.85)
        inputSamples[1][i] *= gain * 0.8;
        */

       //cout << i << " frame: " <<  inputSamples[0][i] << " " << inputSamples[1][i] << " "  << S_max << endl;


      /*
      if(abs(inputSamples[0][i]) > 0 && abs(inputSamples[0][i]) < 100)
        inputSamples[0][i] = 0;
      if(abs(inputSamples[1][i]) > 0 && abs(inputSamples[1][i]) < 100)
        inputSamples[1][i] = 0;
  

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

    }
  
    */

    
    float sum = 0;
    //limit the rms of the entire buffer
    for(int i = 0; i < capture_frames; i++){
		  sum += inputSamples[0][i] * inputSamples[0][i];
	   	sum += inputSamples[1][i] * inputSamples[1][i];
    }

    float currentRms = sqrt( sum/(capture_size));
    //cout<<currentRms<<endl;

    int rms = 1;
    //nerfing with rms
    if(rms){
		float rmsDiff = targetRms - currentRms;
		if(rmsDiff != 0){
			for(int i = 0; i < capture_frames; i++){
				inputSamples[0][i] *= (1.0 + rmsDiff) * (1.0 + rmsDiff);
				inputSamples[1][i] *= (1.0 + rmsDiff) * (1.0 + rmsDiff);

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


			}
		}


	}


    //printf("%f\n", gain);

    //smoothen the gain


    //limit the rms of the entire buffer
    //currentRms = sqrt(    );

    //convert from 1.0 to -1 back to 2byte units
    //little endian

    char output[capture_size];

    for(int i = 0; i < capture_frames; i++){
      //left
      *(signed short*)temp = inputSamples[0][i] * 32768;
      
      output[i*4] = temp[0];
      output[i*4+1] = temp[1];

      //right
      *(signed short*)temp = inputSamples[1][i] * 32768;
      
      output[i*4+2] = temp[0];
      output[i*4+3] = temp[1];

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

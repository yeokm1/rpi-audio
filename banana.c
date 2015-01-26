
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

void SetAlsaMasterVolume(long volume, char* card, char* selem_name)
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    //const char *card = "default";
    //const char *selem_name = "Pi";


    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}


int main(int argc, char * argv[]) {

    int i = 0;
    for(i = 0; i < 100; i ++){
        SetAlsaMasterVolume(i,argv[1], argv[2]);
        sleep(0.5);
    }

}





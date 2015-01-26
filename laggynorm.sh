#!/bin/bash

cd ~/DynamicAudioNormalizer-master/DynamicAudioNormalizerCLI/bin

export SOX_OPTS="−−buffer 1"
export AUDIODEV=hw:1,0
export AUDIODRIVER=alsa

sox -d -t raw -b 16 -e signed -r 44100 -c 2 - | ./DynamicAudioNormalizerCLI -r 0.8 -m 100 -f 10 -g 3 -i - --input-bits 16 --input-chan 2 --input-rate 44100 -o - | sox -t raw -b 16 -e signed -r 44100 -c 2 - -d


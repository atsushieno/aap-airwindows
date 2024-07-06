# aap-airwindows: Airwindows port to AAP

aap-airwindows is an experimental port of [Airwindows](https://github.com/airwindows/airwindows/) to [AAP (Audio Plugins For Android)](https://github.com/atsushieno/aap-core).

It makes full use of [baconpaul/airwin2rack](https://github.com/baconpaul/airwin2rack). Yet, AGP fails to invoke its recursive CMake target, so we rather generate nessessities by our own (we need to generate our own `aap_metadata.xml` anyway).

## License

aap-airwindows is released under the MIT license.

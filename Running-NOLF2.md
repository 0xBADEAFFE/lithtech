To Run any current builds of Lithtech
-------------------------------------

* install DXVK-native (1.9.x) see [build_dxvk-native](https://gitlab.com/Katana-Steel/lith_docker/-/blob/master/build_dxvk.sh) or [get a release build](https://github.com/Joshua-Ashton/dxvk-native/releases)
* grab a recent build from [Gitlab Pipelines](https://gitlab.com/Katana-Steel/lithtech/-/pipelines)
  * any would do, except the no dxvk (that is still missing the OpenGL renderer)
* extract the archive
* grab the `tests/create_nolf2.sh` script
* run it like this: `tests/create_nolf2.sh ~/Games/NOLF2-location build`
* copy and/or link **rez** files from your NOLF2 install,
* copy **cfg** files, namely autoexec.cfg (Lithtech currently aren't creating sane defaults)
* copy **profiles** folder from your NOLF2 install
* finally run `./Lithtech`

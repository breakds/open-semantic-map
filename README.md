# open-semantic-map
A (currently) experimental repo for Open Street Map based algorithm library. 

## Interact with Open Street Map Files (PBF)

OSM `pbf` files are nothing but protocol buffer records. The tool
[osmium](https://osmcode.org/file-formats-manual/) can be used to
interact with them.

To interact with the data programmatically, we use the c++ library
[libosmium](https://osmcode.org/osmium-concepts/) and its python
bindings `PyOsmium`.

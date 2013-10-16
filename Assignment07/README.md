Assignment07 -- Texture Loading
===============================
Cameron Rowe is my partner for this project.

Texture loading complete. Model will load if texture file is stated in object file or .mtl file.

### Compilation Notes ###
libfreeimage-dev required to compile this project.

### Problems ###
If a model contains multiple textures it will only use the last loaded texture.

## Command Line Arguments ##
* -m modelFile -- the .obj file to be loaded
* -v vertexShader -- the vertex shader to be loaded
* -f fragmentShader -- the fragment shader to be loaded

## Interaction Options ##
Menu options are as follows:

* Pause Simulation
* Reverse orbit
* Reverse Rotation
* Exit Program

If the user clicks the window, the rotation will be reversed.

Keyboard Options:

* A Key -- Reverse orbit of the planets
* Left Arrow Key -- Make planets orbit clockwise
* Right Arrow Key -- Make planets orbit counter-clockwise

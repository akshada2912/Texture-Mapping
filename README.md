# Texture Mapping

Implemented texture mapping in the renderer. Each object in the scene may have a texture attached to it, describing the diffuse color c(x) at point x. If the texture is missing for an object, then a constant diffuse color will be assigned to the object.

Implemented barycentric interpolation to fetch the uv coordinates at the intersection point and stored this in the Interaction struct.
1. Nearest Neighbour fetch of textures
Implemented a new function Texture::nearestNeighbourFetch in texture.cpp which takes the uv coordinates as input and returns the color according to nearest neighbor interpolation strategy.
2. Bi-linear interpolation for texture fetches
Implemented a new function Texture::bilinearFetch in texture.cpp which takes the uv coordinates as input and returns the color according to the bilinear neighbor interpolation strategy. Finally, used the color obtained from the above functions in the rendering loop to replace the c(x)
by the value returned from the texture lookup.
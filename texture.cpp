#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

Texture::Texture(std::string pathToImage)
{
    size_t pos = pathToImage.find(".exr");

    if (pos > pathToImage.length())
    {
        this->type = TextureType::UNSIGNED_INTEGER_ALPHA;
        pos = pathToImage.find(".png");

        if (pos > pathToImage.length())
            this->loadJpg(pathToImage);
        else
            this->loadPng(pathToImage);
    }
    else
    {
        this->type = TextureType::FLOAT_ALPHA;
        this->loadExr(pathToImage);
    }
}

void Texture::allocate(TextureType type, Vector2i resolution)
{
    this->resolution = resolution;
    this->type = type;

    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA)
    {
        uint32_t *dpointer = (uint32_t *)malloc(this->resolution.x * this->resolution.y * sizeof(uint32_t));
        this->data = (uint64_t)dpointer;
    }
    else if (this->type == TextureType::FLOAT_ALPHA)
    {
        float *dpointer = (float *)malloc(this->resolution.x * this->resolution.y * 4 * sizeof(float));
        this->data = (uint64_t)dpointer;
    }
}

void Texture::writePixelColor(Vector3f color, int x, int y)
{
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA)
    {
        uint32_t *dpointer = (uint32_t *)this->data;

        uint32_t r = static_cast<uint32_t>(std::min(color.x * 255.0f, 255.f));
        uint32_t g = static_cast<uint32_t>(std::min(color.y * 255.0f, 255.f)) << 8;
        uint32_t b = static_cast<uint32_t>(std::min(color.z * 255.0f, 255.f)) << 16;
        uint32_t a = 255 << 24;

        uint32_t final = r | g | b | a;

        dpointer[y * this->resolution.x + x] = final;
    }
}

/*
Reads the color defined at integer coordinates 'x,y'.
The top left corner of the texture is mapped to '0,0'.
*/
Vector3f Texture::loadPixelColor(int x, int y)
{
    Vector3f rval(0.f, 0.f, 0.f);
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA)
    {
        uint32_t *dpointer = (uint32_t *)this->data;

        uint32_t val = dpointer[y * this->resolution.x + x];
        uint32_t r = (val >> 0) & 255u;
        uint32_t g = (val >> 8) & 255u;
        uint32_t b = (val >> 16) & 255u;
        // printf("%d %d %d\n",r,g,b);
        rval.x = r / 255.f;
        rval.y = g / 255.f;
        rval.z = b / 255.f;
    }

    return rval;
}

template <typename T>
constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (hi < v) ? hi
                                    : v;
}

Vector3f Texture::nearestNeighbourFetch(Vector2f &uv, int width, int height, float t_x_prime, float t_y_prime)
{
    // uv[0]=uv[0]*width;
    // uv[1]=uv[1]*height;
    //printf("%f %f\n",uv[0]*width,uv[1]*height);

    // int tx2=round(uv[0]);
    // int ty2=round(1-uv[1]);
    int tx=round(uv[0]*(width-1));
    int ty=round(uv[1]*(height-1));

    if(tx>(width-1) || ty>(height-1) || tx<0 || ty <0)
    return Vector3f(0.f,0.f,0.f);
    //printf("%d %d\n",tx,ty);
    //uint32_t *dpointer = (uint32_t *)this->data;
   // uint32_t val = dpointer[ty * this->resolution.x + tx];
   // printf("%d\n",val);
    // // Use loadPixelColor to get the color of the nearest neighbor
     Vector3f nearestColor = loadPixelColor(tx, ty);

    return nearestColor;
    return Vector3f(1.f,1.f,1.f);
}

Vector3f Texture::BilinearFetch(Vector2f &uv, int width, int height, float t_x_prime, float t_y_prime)
{
    // float x = uv[0] * (width - 1);
    // float y = uv[1] * (height - 1);

    // // Round and clamp the coordinates to get the nearest integer pixel
    // int ix = clamp(static_cast<int>(std::round(x)), 0, width - 1);
    // int iy = clamp(static_cast<int>(std::round(y)), 0, height - 1);

    // int ix2 = static_cast<int>(std::round(x));
    // int iy2 = static_cast<int>(std::round(y));

    // int tx = clamp(static_cast<int>(std::round(uv.x)), 0, width - 1);
    // int ty = clamp(static_cast<int>(std::round(1 - uv.y)), 0, height - 1);

    // int tx2 = static_cast<int>(std::round(uv.x));
    // int ty2 = static_cast<int>(std::round(1 - uv.y));
    float tx=(uv[0]*(width-1));
    float ty=(uv[1]*(height-1));

    if(tx>(width-1) || ty>(height-1) || tx<0 || ty <0)
    return Vector3f(0.f,0.f,0.f);

    int x1 = std::floor(tx); 
    int x2 = std::ceil(tx);  
    int y1 = std::floor(ty); 
    int y2 = std::ceil(ty);  
    //printf("%d %d %d %d\n",x1,x2,y1,y2);
    // Get the four neighboring pixel colors
    // Vector3f c1 = Texture::loadPixelColor(x1, y1);
    // Vector3f c2 = Texture::loadPixelColor(x2, y1);
    // Vector3f c3 = Texture::loadPixelColor(x1, y2);
    // Vector3f c4 = Texture::loadPixelColor(x2, y2);
    //Vector3f temp=Texture::loadPixelColor(1024,1024);
    Vector3f c3 = Texture::loadPixelColor(x1, y1);
    Vector3f c4 = Texture::loadPixelColor(x2, y1);
    Vector3f c1 = Texture::loadPixelColor(x1, y2);
    Vector3f c2 = Texture::loadPixelColor(x2, y2);
    //cu=(x2-tx)c1 + (tx-x1)c2
    //cl=(x2-tx)c3 + (tx-x1)c4
    //c=(y2-ty)cu + (ty-y1)cl


    // float fx = tx - x1;
    // float fy = ty - y1;
    // float fx2 = tx - x2;
    // float fy2 = ty - y2;
    Vector3f cu=(x2-tx)*c1 + (tx-x1)*c2;
    Vector3f cl=(x2-tx)*c3 + (tx-x1)*c4;
    Vector3f color= (ty-y1)*cu+(y2-ty)*cl;
    return color;


    // Vector3f interpolatedColor = (1 - fx) * (1 - fy) * c1 + fx * (1 - fy) * c2 + (1 - fx) * fy * c3 + fx * fy * c4;
    // //Vector3f interpolatedColor = (1 - fx2) * (1 - fy2) * c1 + fx * (1 - fy2) * c2 + (1 - fx2) * fy * c3 + fx * fy * c4;

    // //Vector3f interpolatedColor = (x1-tx2) * (y1-ty2) * c1 + (tx2-x1) * (y1-ty2) * c2 + (x1-tx2) * (ty2-y1) * c3 + (tx2-x1) * (ty2-y1) * c4;

    // return interpolatedColor;
}

void Texture::loadJpg(std::string pathToJpg)
{
    Vector2i res;
    int comp;
    unsigned char *image = stbi_load(pathToJpg.c_str(), &res.x, &res.y, &comp, STBI_rgb_alpha);
    int textureID = -1;
    if (image)
    {
        this->resolution = res;
        this->data = (uint64_t)image;

        /* iw - actually, it seems that stbi loads the pictures
            mirrored along the y axis - mirror them here */
        for (int y = 0; y < res.y / 2; y++)
        {
            uint32_t *line_y = (uint32_t *)this->data + y * res.x;
            uint32_t *mirrored_y = (uint32_t *)this->data + (res.y - 1 - y) * res.x;
            int mirror_y = res.y - 1 - y;
            for (int x = 0; x < res.x; x++)
            {
                std::swap(line_y[x], mirrored_y[x]);
            }
        }
    }
    else
    {
        std::cerr << "Could not load .jpg texture from " << pathToJpg << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        exit(1);
    }
}

void Texture::loadPng(std::string pathToPng)
{
    Vector2i res;
    int comp;
    unsigned char *image = stbi_load(pathToPng.c_str(), &res.x, &res.y, &comp, STBI_rgb_alpha);
    int textureID = -1;
    if (image)
    {
        this->resolution = res;
        this->data = (uint64_t)image;

        /* iw - actually, it seems that stbi loads the pictures
            mirrored along the y axis - mirror them here */
        for (int y = 0; y < res.y / 2; y++)
        {
            uint32_t *line_y = (uint32_t *)this->data + y * res.x;
            uint32_t *mirrored_y = (uint32_t *)this->data + (res.y - 1 - y) * res.x;
            int mirror_y = res.y - 1 - y;
            for (int x = 0; x < res.x; x++)
            {
                std::swap(line_y[x], mirrored_y[x]);
            }
        }
    }
    else
    {
        std::cerr << "Could not load .png texture from " << pathToPng << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        exit(1);
    }
}

void Texture::loadExr(std::string pathToExr)
{
    int width;
    int height;
    const char *err = nullptr; // or nullptr in C++11

    float *data;
    int ret = LoadEXR(&data, &width, &height, pathToExr.c_str(), &err);
    this->data = (uint64_t)data;

    if (ret != TINYEXR_SUCCESS)
    {
        std::cerr << "Could not load .exr texture map from " << pathToExr << std::endl;
        exit(1);
    }
    else
    {
        this->resolution = Vector2i(width, height);
    }
}

void Texture::save(std::string path)
{
    size_t pos = path.find(".png");

    if (pos > path.length())
    {
        this->saveExr(path);
    }
    else
    {
        this->savePng(path);
    }
}

void Texture::saveExr(std::string path)
{
    if (this->type == TextureType::FLOAT_ALPHA)
    {
        uint64_t hostData = this->data;

        const char *err = nullptr;
        SaveEXR((float *)hostData, this->resolution.x, this->resolution.y, 4, 0, path.c_str(), &err);

        if (err == nullptr)
            std::cout << "Saved EXR: " << path << std::endl;
        else
            std::cerr << "Could not save EXR: " << err << std::endl;
    }
    else
    {
        std::cerr << "Cannot save to EXR: texture is not of type float." << std::endl;
    }
}

void Texture::savePng(std::string path)
{
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA)
    {
        uint64_t hostData = this->data;
        const uint32_t *data = (const uint32_t *)hostData;

        std::vector<uint32_t> pixels;
        for (int y = 0; y < this->resolution.y; y++)
        {
            const uint32_t *line = data + (this->resolution.y - 1 - y) * this->resolution.x;
            for (int x = 0; x < this->resolution.x; x++)
            {
                pixels.push_back(line[x] | (0xff << 24));
            }
        }

        stbi_write_png(path.c_str(), this->resolution.x, this->resolution.y, 4, data, this->resolution.x * sizeof(uint32_t));

        std::cout << "Saved PNG: " << path << std::endl;
    }
    else
    {
        std::cerr << "Cannot save to PNG: texture is not of type uint32." << std::endl;
    }
}
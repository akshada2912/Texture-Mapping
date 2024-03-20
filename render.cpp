#include "render.h"
#include "light.cpp"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

// long long Integrator::render2()
// {
//     auto startTime = std::chrono::high_resolution_clock::now();
//     for (int x = 0; x < this->scene.imageResolution.x; x++) {
//         for (int y = 0; y < this->scene.imageResolution.y; y++) {
//             Ray cameraRay = this->scene.camera.generateRay(x, y);
//             Interaction si = this->scene.rayIntersect(cameraRay);

//             if (si.didIntersect)
//                 this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
//             else
//                 this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
//         }
//     }
//     auto finishTime = std::chrono::high_resolution_clock::now();

//     return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
// }

float dotProduct(const Vector3f &a, const Vector3f &b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float dotProduct2(const Vector2f &a, const Vector2f &b)
{
    return a[0] * b[0] + a[1] * b[1];
}

Vector3f crossProduct3D(const Vector3f &v1, const Vector3f &v2)
{
    return {
        v1[1] * v2[2] - v1[2] * v2[1],
        v1[2] * v2[0] - v1[0] * v2[2],
        v1[0] * v2[1] - v1[1] * v2[0]};
}

float calculateTriangleArea(const Vector3f &v1, const Vector3f &v2, const Vector3f &v3)
{
    Vector3f v21 = v2 - v1;
    Vector3f v31 = v3 - v1;

    Vector3f crossProductResult = crossProduct3D(v21, v31);

    float area = 0.5f * std::sqrt(crossProductResult[0] * crossProductResult[0] + crossProductResult[1] * crossProductResult[1] + crossProductResult[2] * crossProductResult[2]);
    return Cross(v1-v2,v1-v3).Length()/2;
    return area;
}

Vector3f getIllumination(const Vector3f &p, const Light &light)
{
    Vector3f lightDirection;

    // Point light
    lightDirection = Vector3f(light.pointLight.location[0] - p[0],
                              light.pointLight.location[1] - p[1],
                              light.pointLight.location[2] - p[2]);

    float distance = std::sqrt(lightDirection[0] * lightDirection[0] +
                               lightDirection[1] * lightDirection[1] +
                               lightDirection[2] * lightDirection[2]);

    float attenuation = 1.0f / distance;

    return Vector3f(light.pointLight.radiance[0], light.pointLight.radiance[1], light.pointLight.radiance[2]) * attenuation;
}

long long Integrator::render3()
{
    std::vector<Light> lights = loadLights(this->scene.config);
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int x = 0; x < this->scene.imageResolution.x; x++)
    {
        for (int y = 0; y < this->scene.imageResolution.y; y++)
        {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if (si.didIntersect)
            {

                // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                Vector3f pixelColor(0.f, 0.f, 0.f);

                // Iterate over each directional light in the scene
                for (const auto &directionalLight : lights)
                {
                    if (directionalLight.type == LightType::DIRECTIONAL_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.directionalLight.direction[0],
                            directionalLight.directionalLight.direction[1],
                            directionalLight.directionalLight.direction[2]};

                        Ray shadowRay(si.p + 1e-5 * si.n, lightDirection);
                        Interaction shadowHit = this->scene.rayIntersect(shadowRay);

                        if (!shadowHit.didIntersect)
                        {
                            Vector3f brdfTerm = Vector3f(1.0f, 1.0f, 1.0f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);

                            // pixelColor += brdfTerm * dotProduct(lightDirection, si.n);
                            // brdf * (w.n)* li(x,w)
                            pixelColor += brdfTerm * dotProduct(lightDirection, si.n) * Vector3f(directionalLight.directionalLight.radiance[0], directionalLight.directionalLight.radiance[1], directionalLight.directionalLight.radiance[2]);
                        }
                    }

                    if (directionalLight.type == LightType::POINT_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.pointLight.location[0] - si.p[0],
                            directionalLight.pointLight.location[1] - si.p[1],
                            directionalLight.pointLight.location[2] - si.p[2]};

                        float distanceToPoint = std::sqrt(lightDirection[0] * lightDirection[0] + lightDirection[1] * lightDirection[1] + lightDirection[2] * lightDirection[2]);
                        // float distanceToPoint = std::sqrt((directionalLight.pointLight.location[0] - si.p[0]) * (directionalLight.pointLight.location[0] - si.p[0]) + (directionalLight.pointLight.location[1] - si.p[1]) * (directionalLight.pointLight.location[1] - si.p[1]) + (directionalLight.pointLight.location[2] - si.p[2]) * (directionalLight.pointLight.location[2] - si.p[2]));
                        // if (distanceToPoint > 0.0f)
                        // {
                        //     lightDirection /= distanceToPoint;
                        // } // makes ray p-x/|p-x|
                        // lightDirection=Normalize(lightDirection);
                        // if(dotProduct(si.n,lightDirection/distanceToPoint)<0)
                        
                        Ray shadowray = Ray(si.p + 1e-3 * si.n, lightDirection / distanceToPoint);
                        Interaction shadowHit = this->scene.rayIntersect(shadowray);
                        if(Dot(si.n,Normalize(lightDirection))<0)
                        continue;
                        if (  shadowHit.t > lightDirection.Length())
                        {
                            // if(Dot(si.n,Normalize(lightDirection))<0)
                            // si.n=-1*si.n;
                            if(Dot(cameraRay.d, si.n)>0)
                            si.n=-1*si.n;
                            Vector3f brdfTerm = Vector3f(1.f, 1.f, 1.f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                            Vector3f Li = {
                                (directionalLight.pointLight.radiance[0]),
                                (directionalLight.pointLight.radiance[1]),
                                (directionalLight.pointLight.radiance[2])}; // r^2
                            
                    
                            // light diminishes by inverse square law
                            //  brdf * (w.n)* li(x,w)
                           // if(dotProduct(lightDirection/distanceToPoint,si.n)>=0)
                                pixelColor += brdfTerm * (Dot(si.n,lightDirection) / distanceToPoint) * Li / (distanceToPoint * distanceToPoint);
                            //si.n=-si.n;
                        }

                        // double lightDirection2[3];
                        // lightDirection2[0] = directionalLight.pointLight.location2[0] - si.p[0];
                        // lightDirection2[1] = directionalLight.pointLight.location2[1] - si.p[1];
                        // lightDirection2[2] = directionalLight.pointLight.location2[2] - si.p[2];

                        // double distanceToPoint = std::sqrt(lightDirection2[0] * lightDirection2[0] + lightDirection2[1] * lightDirection2[1] + lightDirection2[2] * lightDirection2[2]);
                        // // float distanceToPoint = std::sqrt((directionalLight.pointLight.location[0] - si.p[0]) * (directionalLight.pointLight.location[0] - si.p[0]) + (directionalLight.pointLight.location[1] - si.p[1]) * (directionalLight.pointLight.location[1] - si.p[1]) + (directionalLight.pointLight.location[2] - si.p[2]) * (directionalLight.pointLight.location[2] - si.p[2]));
                        // // if (distanceToPoint > 0.0f)
                        // // {
                        // //     lightDirection2 /= distanceToPoint;
                        // // } // makes ray p-x/|p-x|
                        // // lightDirection2=Normalize(lightDirection2);
                        // Ray shadowray = Ray(si.p + 1e-3 * si.n, Vector3f(lightDirection2[0],lightDirection2[1],lightDirection2[2]) / distanceToPoint);
                        // Interaction shadowHit = this->scene.rayIntersect(shadowray);

                        // if (!shadowHit.didIntersect)
                        // {
                        //     Vector3f brdfTerm = Vector3f(1.f, 1.f, 1.f) / M_PI;
                        //     // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                        //     Vector3f Li = {
                        //         (directionalLight.pointLight.radiance[0]),
                        //         (directionalLight.pointLight.radiance[1]),
                        //         (directionalLight.pointLight.radiance[2])}; // r^2

                        //     // light diminishes by inverse square law
                        //     //  brdf * (w.n)* li(x,w)

                        //     pixelColor += brdfTerm * (((si.n.x * lightDirection2[0] + si.n.y * lightDirection2[1] + si.n.z * lightDirection2[2])) / distanceToPoint) * Li / (distanceToPoint * distanceToPoint);
                        // }
                    }
                }

                // Write the final pixel color to the output image
                this->outputImage.writePixelColor(pixelColor, x, y);
            }
            else
            {
                // No intersection, write black color
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
            }
        }
    }

    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

long long Integrator::render()
{
    std::vector<Light> lights = loadLights(this->scene.config);
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int x = 0; x < this->scene.imageResolution.x; x++)
    {
        for (int y = 0; y < this->scene.imageResolution.y; y++)
        {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if (si.didIntersect)
            {

                Surface closestSurface = this->scene.surfaces[0];
                float closestDistance = std::numeric_limits<float>::infinity();
                Vector3f brdfTerm;

                // Iterate over surfaces to find the closest intersection
                for (auto &surface : this->scene.surfaces) // finding surface the pixel is in
                {
                    Interaction surfaceInteraction = surface.rayIntersect(cameraRay);
                    if (surfaceInteraction.didIntersect && surfaceInteraction.t < closestDistance)
                    {
                        closestSurface = surface;
                        closestDistance = surfaceInteraction.t;
                    }
                }

                // if (closestDistance == std::numeric_limits<float>::infinity())
                // {
                //    // printf("%f %f\n",si.p.x,si.p.y);
                // }
                // else
                // {
                if (si.hasres==1)
                {
                    Interaction s12;
                    Tri tri1;
                    for (auto &tri : closestSurface.tris)
                    {
                        Interaction si2 = closestSurface.rayTriangleIntersect(cameraRay, tri.v1, tri.v2, tri.v3, tri.normal);
                        if (si2.p == si.p && si.n == si2.n && si.t == si2.t && si.didIntersect == si2.didIntersect)
                        {
                            tri1 = tri; // getting triangle the pixel is in
                            break;
                        }
                    }
                    Vector2f uv1 = tri1.uv1;
                    Vector2f uv2 = tri1.uv2;
                    Vector2f uv3 = tri1.uv3;
                    Vector3f v1 = tri1.v1;
                    Vector3f v2 = tri1.v2;
                    Vector3f v3 = tri1.v3;
                    //printf("%f %f\n%f %f\n %f %f\n",tri1.uv1.x, tri1.uv1.y,tri1.uv2.x, tri1.uv2.y,tri1.uv3.x, tri1.uv3.y);
                    float areaV1V2V3 = calculateTriangleArea(si.v1, si.v2, si.v3);
                    float areaPV2V3 = calculateTriangleArea(si.p, si.v2, si.v3);

                    // float alpha = areaPV2V3 / areaV1V2V3;

                    float areaPV1V3 = calculateTriangleArea(si.p, si.v1, si.v3);
                    float alpha = areaPV2V3 / areaV1V2V3;
                    float beta = areaPV1V3 / areaV1V2V3;
                    float gamma = 1 - alpha - beta;

                    Vector2f bary = alpha * si.uv1 + beta * si.uv2 + gamma * si.uv3;
                    // printf("%f %f\n",bary[0],bary[1]);

                    si.uv = bary;

                    // if (areaV1V2V3 == 0)
                    // {
                    //     si.uv = Vector2f(0.f, 0.f);
                    //     Vector3f color = Vector3f(1.f, 1.f, 1.f);
                    //     brdfTerm = color / M_PI;
                    // }
                    // else
                    // {
                        // if (areaPV1V3 == 0)
                        //     si.uv = (tri1.uv1 + tri1.uv3) / 2;
                        // if (areaPV2V3 == 0)
                        //     si.uv = (tri1.uv2 + tri1.uv3) / 2;
                        Vector3f color = this->scene.surfaces[si.s_ind].diffuseTexture.nearestNeighbourFetch(si.uv, si.resolution.x, si.resolution.y, si.txprime, si.typrime);
                        //Vector3f color=Vector3f(0.f,0.f,0.f);
                        // Vector3f color = closestSurface.diffuseTexture.BilinearFetch(si.uv, closestSurface.diffuseTexture.resolution.x, closestSurface.diffuseTexture.resolution.y, si.txprime, si.typrime);
                        brdfTerm = color / M_PI;
                    //}
                    // Vector3f color=Vector3f(1.f,1.f,1.f);

                    // printf("%f %f %f\n", color[0], color[1], color[2]);
                }
                else
                {
                    brdfTerm = this->scene.surfaces[si.s_ind].diffuse / M_PI;
                    // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                }
                //}
                // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                Vector3f pixelColor(0.f, 0.f, 0.f);

                // Iterate over each directional light in the scene
                for (const auto &directionalLight : lights)
                {
                    if (directionalLight.type == LightType::DIRECTIONAL_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.directionalLight.direction[0],
                            directionalLight.directionalLight.direction[1],
                            directionalLight.directionalLight.direction[2]};

                        Ray shadowRay(si.p + 1e-5 * si.n, lightDirection);
                        Interaction shadowHit = scene.rayIntersect(shadowRay);

                        if (!shadowHit.didIntersect)
                        {
                            // Vector3f brdfTerm = Vector3f(1.0f, 1.0f, 1.0f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);

                            // pixelColor += brdfTerm * dotProduct(lightDirection, si.n);
                            // brdf * (w.n)* li(x,w)
                            pixelColor += brdfTerm * dotProduct(lightDirection, si.n) * Vector3f(directionalLight.directionalLight.radiance[0], directionalLight.directionalLight.radiance[1], directionalLight.directionalLight.radiance[2]);
                        }
                    }

                    else if (directionalLight.type == LightType::POINT_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.pointLight.location[0] - si.p[0],
                            directionalLight.pointLight.location[1] - si.p[1],
                            directionalLight.pointLight.location[2] - si.p[2]};

                        float distanceToPoint = std::sqrt(lightDirection[0] * lightDirection[0] + lightDirection[1] * lightDirection[1] + lightDirection[2] * lightDirection[2]);
                        lightDirection /= distanceToPoint; // makes ray p-x/|p-x|

                        Ray shadowRay(si.p + 1e-3f * si.n, lightDirection);
                        Interaction shadowHit = scene.rayIntersect(shadowRay);

                        // if ( shadowHit.t > lightDirection.Length())
                        if (!shadowHit.didIntersect)
                        {
                            if(Dot(cameraRay.d, si.n)>0)
                            si.n=-1*si.n;
                            // Vector3f brdfTerm = Vector3f(1.0f,1.0f, 1.0f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                            Vector3f Li = {
                                (directionalLight.pointLight.radiance[0]),
                                (directionalLight.pointLight.radiance[1]),
                                (directionalLight.pointLight.radiance[2])}; // r^2

                            // light diminishes by inverse square law
                            //  brdf * (w.n)* li(x,w)
                            pixelColor += Li * brdfTerm * dotProduct(lightDirection, si.n) / (distanceToPoint * distanceToPoint);
                        }
                    }
                }

                // Write the final pixel color to the output image
                this->outputImage.writePixelColor(pixelColor, x, y);
            }
            else
            {
                // No intersection, write black color
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
            }
        }
    }

    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

long long Integrator::render2()
{
    std::vector<Light> lights = loadLights(this->scene.config);
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int x = 0; x < this->scene.imageResolution.x; x++)
    {
        for (int y = 0; y < this->scene.imageResolution.y; y++)
        {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if (si.didIntersect)
            {

                Surface closestSurface = this->scene.surfaces[0];
                float closestDistance = std::numeric_limits<float>::infinity();
                Vector3f brdfTerm;

                // Iterate over surfaces to find the closest intersection
                for (auto &surface : this->scene.surfaces) // finding surface the pixel is in
                {
                    Interaction surfaceInteraction = surface.rayIntersect(cameraRay);
                    if (surfaceInteraction.didIntersect && surfaceInteraction.t < closestDistance)
                    {
                        closestSurface = surface;
                        closestDistance = surfaceInteraction.t;
                    }
                }
                if (si.hasres==1)
                {
                    Interaction s12;
                    Tri tri1;
                    for (auto &tri : closestSurface.tris)
                    {
                        Interaction si2 = closestSurface.rayTriangleIntersect(cameraRay, tri.v1, tri.v2, tri.v3, tri.normal);
                        if (si2.p == si.p && si.n == si2.n && si.t == si2.t && si.didIntersect == si2.didIntersect)
                        {
                            tri1 = tri; // getting triangle the pixel is in
                            break;
                        }
                    }
                    Vector2f uv1 = tri1.uv1;
                    Vector2f uv2 = tri1.uv2;
                    Vector2f uv3 = tri1.uv3;
                    Vector3f v1 = tri1.v1;
                    Vector3f v2 = tri1.v2;
                    Vector3f v3 = tri1.v3;
                    float areaV1V2V3 = calculateTriangleArea(si.v1, si.v2, si.v3);
                    float areaPV2V3 = calculateTriangleArea(si.p, si.v2, si.v3);

                    // float alpha = areaPV2V3 / areaV1V2V3;

                     float areaPV1V3 = calculateTriangleArea(si.p, si.v1, si.v3);
                    float alpha = areaPV2V3 / areaV1V2V3;
                    float beta = areaPV1V3 / areaV1V2V3;
                    float gamma = 1 - alpha - beta;

                    Vector2f bary = alpha * si.uv1 + beta * si.uv2 + gamma * si.uv3;

                    float t_x_prime = bary.x;        // ux'
                    float t_y_prime = 1.0f - bary.y; // 1-uy'
                    
                    si.uv = bary;
                    
                    si.txprime = t_x_prime;
                    si.typrime = t_y_prime;
                    // Vector3f color = closestSurface.diffuseTexture.nearestNeighbourFetch(si.uv, closestSurface.diffuseTexture.resolution.x, closestSurface.diffuseTexture.resolution.y, si.txprime, si.typrime);

                    Vector3f color = this->scene.surfaces[si.s_ind].diffuseTexture.BilinearFetch(si.uv, si.resolution.x, si.resolution.y, si.txprime, si.typrime);
                    brdfTerm = color / M_PI;
                    
                    // printf("%f %f %f\n", color[0], color[1], color[2]);
                }
                else
                {
                    brdfTerm = closestSurface.diffuse / M_PI;
                    // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                }
                // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                Vector3f pixelColor(0.f, 0.f, 0.f);

                // Iterate over each directional light in the scene
                for (const auto &directionalLight : lights)
                {
                    if (directionalLight.type == LightType::DIRECTIONAL_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.directionalLight.direction[0],
                            directionalLight.directionalLight.direction[1],
                            directionalLight.directionalLight.direction[2]};

                        Ray shadowRay(si.p + 1e-5 * si.n, lightDirection);
                        Interaction shadowHit = scene.rayIntersect(shadowRay);

                        if (!shadowHit.didIntersect)
                        {
                            // Vector3f brdfTerm = Vector3f(1.0f, 1.0f, 1.0f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);

                            // pixelColor += brdfTerm * dotProduct(lightDirection, si.n);
                            // brdf * (w.n)* li(x,w)
                            pixelColor += brdfTerm * dotProduct(lightDirection, si.n) * Vector3f(directionalLight.directionalLight.radiance[0], directionalLight.directionalLight.radiance[1], directionalLight.directionalLight.radiance[2]);
                        }
                    }

                    else if (directionalLight.type == LightType::POINT_LIGHT)
                    {
                        Vector3f lightDirection = {
                            directionalLight.pointLight.location[0] - si.p[0],
                            directionalLight.pointLight.location[1] - si.p[1],
                            directionalLight.pointLight.location[2] - si.p[2]};

                        float distanceToPoint = std::sqrt(lightDirection[0] * lightDirection[0] + lightDirection[1] * lightDirection[1] + lightDirection[2] * lightDirection[2]);
                        lightDirection /= distanceToPoint; // makes ray p-x/|p-x|

                        Ray shadowRay(si.p + 1e-3f * si.n, lightDirection);
                        Interaction shadowHit = scene.rayIntersect(shadowRay);

                        // if ( shadowHit.t > lightDirection.Length())
                        if (!shadowHit.didIntersect)
                        {
                            // Vector3f brdfTerm = Vector3f(1.0f,1.0f, 1.0f) / M_PI;
                            // printf("%f %f %f\n", brdfTerm[0], brdfTerm[1], brdfTerm[2]);
                            Vector3f Li = {
                                (directionalLight.pointLight.radiance[0]),
                                (directionalLight.pointLight.radiance[1]),
                                (directionalLight.pointLight.radiance[2])}; // r^2

                            // light diminishes by inverse square law
                            //  brdf * (w.n)* li(x,w)
                            pixelColor += Li * brdfTerm * dotProduct(lightDirection, si.n) / (distanceToPoint * distanceToPoint);
                        }
                    }
                }

                // Write the final pixel color to the output image
                this->outputImage.writePixelColor(pixelColor, x, y);
            }
            else
            {
                // No intersection, write black color
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
            }
        }
    }

    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./render <scene_config> <out_path> <texture_choice>";
        return 1;
    }
    Scene scene(argv[1]);
    int choice = std::stoi(argv[3]);

    Integrator rayTracer(scene);
    auto renderTime = 0;
    if (choice == 0)
        renderTime = rayTracer.render();
    else if (choice == 1)
        renderTime = rayTracer.render2();
    else if (choice == 2) // section 2
        renderTime = rayTracer.render3();

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}

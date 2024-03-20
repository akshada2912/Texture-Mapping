#include "light.h"

using json = nlohmann::json;

std::vector<Light> loadLights(const json& sceneConfig) {
    std::vector<Light> lights;

    // Load point lights
    if (sceneConfig.contains("pointLights")) {
        nlohmann::json pointLights = sceneConfig["pointLights"];
        for (const auto& pointLightData : pointLights) {
            Light light;
            light.type = LightType::POINT_LIGHT;
            // light.pointLight.location.push_back({
            //     pointLightData["location"][0],
            //     pointLightData["location"][1],
            //     pointLightData["location"][2]
            // });
            light.pointLight.location[0] = pointLightData["location"][0];
            light.pointLight.location[1] = pointLightData["location"][1];
            light.pointLight.location[2] = pointLightData["location"][2];

            light.pointLight.location2[0] = pointLightData["location"][0];
            light.pointLight.location2[1] = pointLightData["location"][1];
            light.pointLight.location2[2] = pointLightData["location"][2];

            light.pointLight.radiance[0] = pointLightData["radiance"][0];
            light.pointLight.radiance[1] = pointLightData["radiance"][1];
            light.pointLight.radiance[2] = pointLightData["radiance"][2];

            light.pointLight.radiance2[0] = pointLightData["radiance"][0];
            light.pointLight.radiance2[1] = pointLightData["radiance"][1];
            light.pointLight.radiance2[2] = pointLightData["radiance"][2];

            lights.push_back(light);
        }
    }

    // Load directional lights
    if (sceneConfig.find("directionalLights") != sceneConfig.end()) {
        const auto& directionalLights = sceneConfig["directionalLights"];
        for (const auto& directionalLightData : directionalLights) {
            Light light;
            light.type = LightType::DIRECTIONAL_LIGHT;

            light.directionalLight.direction[0] = directionalLightData["direction"][0];
            light.directionalLight.direction[1] = directionalLightData["direction"][1];
            light.directionalLight.direction[2] = directionalLightData["direction"][2];

            light.directionalLight.radiance[0] = directionalLightData["radiance"][0];
            light.directionalLight.radiance[1] = directionalLightData["radiance"][1];
            light.directionalLight.radiance[2] = directionalLightData["radiance"][2];

            lights.push_back(light);
        }
    }

    return lights;
}
